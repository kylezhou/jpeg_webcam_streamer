/*
 Copyright (C) 2015, Kyle Zhou <kyle.zhou at live.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstring> /* memset() etc. */

#include "JpegFrameParser.hh"

#ifndef NDEBUG
#include <cstdio>
#define LOGGY(format, ...) fprintf (stderr, format, ##__VA_ARGS__)
#else
#define LOGGY(format, ...) 
#endif /* NDEBUG */


enum
{
    START_MARKER = 0xFF,
    SOI_MARKER   = 0xD8,
    JFIF_MARKER  = 0xE0,
    CMT_MARKER   = 0xFE,
    DQT_MARKER   = 0xDB,
    SOF_MARKER   = 0xC0,
    DHT_MARKER   = 0xC4,
    SOS_MARKER   = 0xDA,
    EOI_MARKER   = 0xD9,
    DRI_MARKER   = 0xDD
};

typedef struct
{
    unsigned char id;
    unsigned char samp;
    unsigned char qt;
} CompInfo;


JpegFrameParser::JpegFrameParser() :
    _width(0), _height(0), _type(0),
    _precision(0), _qFactor(255),
    _qTables(NULL), _qTablesLength(0),
    _restartInterval(0),
    _scandata(NULL), _scandataLength(0)
{
    _qTables = new unsigned char[128 * 2];
    memset(_qTables, 8, 128 * 2);
}

JpegFrameParser::~JpegFrameParser()
{
    if (_qTables != NULL)
        delete[] _qTables;
}

/*! \brief Scan and return a JPEG marker
 *  This function simply scans through the data for the first 0xff, and return the byte that follows 0xff.
 *  \param data The JPEG data buffer
 *  \param size The total length of data buffer
 *  \param offset The current position (should be updated once reading completed)
 *  \return The marker (one byte only without 0xFF)
 */
unsigned int JpegFrameParser::scanJpegMarker(const unsigned char* data,
                    unsigned int size, unsigned int* offset)
{
    while ((data[(*offset)++] != START_MARKER) && ((*offset) < size));
    
    if ((*offset) >= size) { // no more data left
        return EOI_MARKER;
    } else {
        unsigned int marker;
        
        marker = data[*offset];
        (*offset)++;
        
        return marker;
    }
}

/*! \brief Get the header size
 *  This function simply calculate the size from the two bytes in data[offset] and data[offset+1], MSB first.
 *  \param data The JPEG data buffer
 *  \param offset The current position (it is not updated by this function)
 *  \return The header size
 */
static unsigned int _jpegHeaderSize(const unsigned char* data, unsigned int offset)
{
    return data[offset] << 8 | data[offset + 1];
}

/*! \brief Read the JFIF marker segment
 *
 *  \param data The JPEG data buffer
 *  \param size The total length of data buffer
 *  \param offset The current position (should be updated once reading completed). offset should point to the segment size field when calling this function.
 *  \return status: 0 - success, otherwise failure
 */
int JpegFrameParser::readJFIF(const unsigned char* data, unsigned int size,
                             unsigned int* offset)
{
    unsigned int off, jfif_size;
    unsigned int major, minor;
    unsigned int unit;
    unsigned int xden, yden, xthumb, ythumb;
    
    off = *offset;
    if(off + 16 > size) goto wrong_size;
    jfif_size = _jpegHeaderSize(data, off);
    if(jfif_size < 16) goto wrong_length;
    *offset += jfif_size; // offset points to the next segment marker
    off += 2; // skip size
    if(strncmp((const char*)data+off, "JFIF", 5) != 0)
        goto wrong_id;
    off += 5;
    major = data[off++];
    minor = data[off++];
    unit = data[off++];
    xden = _jpegHeaderSize(data, off);
    off += 2;
    yden = _jpegHeaderSize(data, off);
    off += 2;
    xthumb = data[off++];
    ythumb = data[off++];
    LOGGY("verion=%d.%d, unit=%d, xden=%d, yden=%d, xthumb=%d, ythumb=%d\n", major, minor, unit, xden, yden, xthumb, ythumb);
    return 0;
    
wrong_size:
    LOGGY("Wrong JFIF size\n");
    return -1;
    
wrong_length:
    LOGGY("Wrong JFIF length\n");
    return -1;

wrong_id:
    LOGGY("Wrong JFIF id\n");
    return -1;

}

/*! \brief Read the SOF marker segment
 *
 *  \param data The JPEG data buffer
 *  \param size The total length of data buffer
 *  \param offset The current position (should be updated once reading completed). offset should point to the segment size field when calling this function.
 *  \return status: 0 - success, otherwise failure
 */
int JpegFrameParser::readSOF(const unsigned char* data, unsigned int size,
                             unsigned int* offset)
{
    int i, j;
    CompInfo elem;
    CompInfo info[3] = { {0,}, };
    unsigned int sof_size, off;
    unsigned int width, height, infolen;
    
    off = *offset;
    
    /* we need at least 17 bytes for the SOF */
    if (off + 17 > size) goto wrong_size;
    
    sof_size = _jpegHeaderSize(data, off);
    if (sof_size < 17) goto wrong_length;
    
    *offset += sof_size; // offset points to the next segment
    
    /* skip size */
    off += 2;
    
    /* precision should be 8 */
    if (data[off++] != 8) goto bad_precision;
    
    /* read dimensions */
    height = data[off] << 8 | data[off + 1];
    width = data[off + 2] << 8 | data[off + 3];
    off += 4;
    
    if (height == 0 || height > 2040) goto invalid_dimension;
    if (width == 0 || width > 2040) goto invalid_dimension;
    
    _width = width / 8;
    _height = height / 8;
    
    /* we only support 3 components */
    if (data[off++] != 3) {
        LOGGY("Number of components are %d. Not supported.\n", data[off-1]);
        goto bad_components;
    }
    
    infolen = 0;
    for (i = 0; i < 3; i++) {
        elem.id = data[off++];
        elem.samp = data[off++];
        elem.qt = data[off++];
        
        /* insertion sort from the last element to the first */
        for (j = infolen; j > 1; j--) {
            if (info[j - 1].id < elem.id) break;
            info[j] = info[j - 1];
        }
        info[j] = elem;
        infolen++;
    }
    
    /* see that the components are supported */
    if (info[0].samp == 0x21) {
        _type = 0;
    } else if (info[0].samp == 0x22) {
        _type = 1;
    } else {
        LOGGY("Sampling factor of comp0 is %02x. NOT supported.\n", info[0].samp);
        goto invalid_comp;
    }
    
    if (info[1].samp != 0x11) {
        LOGGY("Sampling factor of comp1 is %02x. NOT supported.\n", info[1].samp);
        goto invalid_comp;
    }
    if (info[2].samp != 0x11) {
        LOGGY("Sampling factor of comp2 is %02x. NOT supported.\n", info[2].samp);
        goto invalid_comp;
    }
    if (info[1].qt != info[2].qt) {
        LOGGY("Quantization tables for comp1 (%d) and comp2 (%d) are not equal. NOT supported.\n", info[1].qt, info[2].qt);
        goto invalid_comp;
    }
    
    LOGGY("width=%d, height=%d\n", width, height);
    return 0;
    
    /* ERRORS */
wrong_size:
    LOGGY("Wrong SOF size\n");
    return -1;
    
wrong_length:
    LOGGY("Wrong SOF length\n");
    return -1;
    
bad_precision:
    LOGGY("Bad precision\n");
    return -1;
    
invalid_dimension:
    LOGGY("Invalid dimension\n");
    return -1;
    
bad_components:
    LOGGY("Bad component\n");
    return -1;
    
invalid_comp:
    LOGGY("Invalid component\n");
    return -1;
}

/*! \brief Read the DQT marker segment
 *
 *  \param data The JPEG data buffer
 *  \param size The total length of data buffer
 *  \param offset The current position (this function does NOT update offset). offset should point to the segment size field when calling this function.
 *  \return new offset that points to the next segment marker
 */
unsigned int JpegFrameParser::readDQT(const unsigned char* data,
                unsigned int size, unsigned int offset)
{
    unsigned int quant_size, tab_size;
    unsigned char prec;
    unsigned char id;
    
    if (offset + 2 > size)
        goto too_small;
    
    quant_size = _jpegHeaderSize(data, offset);
    if (quant_size < 2)
        goto small_quant_size;
    
    /* clamp to available data */
    if (offset + quant_size > size) {
        quant_size = size - offset;
    }
    
    offset += 2;
    quant_size -= 2;
    
    while (quant_size > 0) {
        /* not enough to read the id */
        if (offset + 1 > size)
            break;
        
        id = data[offset] & 0x0f;
        if (id == 15)
            goto invalid_id;
        
        prec = (data[offset] & 0xf0) >> 4;
        if (prec) {
            tab_size = 128;
            _qTablesLength = 128 * 2;
        } else {
            tab_size = 64;
            _qTablesLength = 64 * 2;
        }
        
        /* there is not enough for the table */
        if (quant_size < tab_size + 1)
            goto no_table;
        
        //LOGGY("Copy quantization table: %u\n", id);
        memcpy(&_qTables[id * tab_size], &data[offset + 1], tab_size);
        
        tab_size += 1;
        quant_size -= tab_size;
        offset += tab_size;
    }
    
done:
    return offset + quant_size;
    
    /* ERRORS */
too_small:
    LOGGY("DQT is too small\n");
    return size;
    
small_quant_size:
    LOGGY("Quantization table is too small\n");
    return size;
    
invalid_id:
    LOGGY("Invalid table ID\n");
    goto done;
    
no_table:
    LOGGY("table doesn't exist\n");
    goto done;
}

/*! \brief Read the DRI marker segment
 *
 *  \param data The JPEG data buffer
 *  \param size The total length of data buffer
 *  \param offset The current position (should be updated once reading completed). offset should point to the segment size field when calling this function.
 *  \return status: 0 - success, otherwise failure (or DRI disabled)
 */
int JpegFrameParser::readDRI(const unsigned char* data,
                             unsigned int size, unsigned int* offset)
{
    unsigned int dri_size, off;
    
    off = *offset;
    
    /* we need at least 4 bytes for the DRI */
    if (off + 4 > size)
        goto wrong_size;
    
    dri_size = _jpegHeaderSize(data, off);
    if (dri_size < 4)
        goto wrong_length;
    
    *offset += dri_size; // point to the next segment marker
    off += 2;
    
    _restartInterval = (data[off] << 8) | data[off + 1];
    if(_restartInterval == 0) // restart disabled
        return -1;
    else
        return 0;
    
wrong_size:
    return -1;
    
wrong_length:
    *offset += dri_size;
    return -1;
}

int JpegFrameParser::parse(unsigned char* data, unsigned int size)
{
    _width  = 0;
    _height = 0;
    _type = 0;
    _precision = 0;
    //_qFactor = 0;
    _restartInterval = 0,
    
    _scandata = NULL;
    _scandataLength = 0;
    
    unsigned int offset = 0;
    unsigned int jfifFound = 0;
    unsigned int dqtFound = 0;
    unsigned int sosFound = 0;
    unsigned int sofFound = 0;
    unsigned int driFound = 0;
    unsigned int jpeg_header_size = 0;
    
    while ((sosFound == 0) && (offset < size)) {
        switch (scanJpegMarker(data, size, &offset)) {
            case JFIF_MARKER:
                LOGGY("JFIF marker found!\n");
                if (readJFIF(data, size, &offset) != 0) {
                    goto invalid_format;
                }
                break;
            case CMT_MARKER:
                LOGGY("CMT marker found!\n");
                offset += _jpegHeaderSize(data, offset); // skip
                break;
            case DHT_MARKER:
                LOGGY("DHT marker found!\n");
                offset += _jpegHeaderSize(data, offset); // skip
                break;
            case SOF_MARKER:
                LOGGY("SOF marker found!\n");
                if (readSOF(data, size, &offset) != 0) {
                    goto invalid_format;
                }
                sofFound = 1;
                break;
            case DQT_MARKER:
                LOGGY("DQT marker found!\n");
                offset = readDQT(data, size, offset);
                dqtFound = 1;
                break;
            case SOS_MARKER:
                LOGGY("SOS marker found!\n");
                sosFound = 1;
                jpeg_header_size = offset + _jpegHeaderSize(data, offset);
                break;
            case EOI_MARKER:
                /* EOI reached before SOS!? */
                LOGGY("EOI reached before SOS!?\n");
                break;
            case SOI_MARKER:
                LOGGY("SOI found\n");
                break;
            case DRI_MARKER:
                LOGGY("DRI found\n");
                if (readDRI(data, size, &offset) == 0) {
                    driFound = 1;
                }
                break;
            default:
                break;
        }
    }
    if ((dqtFound == 0) || (sofFound == 0)) {
        goto unsupported_jpeg;
    }
    
    if (_width == 0 || _height == 0) {
        goto no_dimension;
    }
    
    _scandata = data + jpeg_header_size;
    _scandataLength = size - jpeg_header_size;
    
    if (driFound == 1) {
        _type += 64;
    }
    
    return 0;
    
    /* ERRORS */
unsupported_jpeg:
    return -1;
    
no_dimension:
    return -1;
    
invalid_format:
    return -1;
}
