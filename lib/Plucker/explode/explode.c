/* -*- mode: c; indent-tabs-mode: nil; -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>             /* for link */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

/* This code requires the Independent JPEG Group libjpeg library, version 6b or later */
#include <jpeglib.h>

#include <unpluck.h>

#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            1
#endif

#define GET_FUNCTION_CODE_TYPE(x)    (((x)>>3) & 0x1F)
#define GET_FUNCTION_CODE_DATALEN(x) ((x) & 0x7)

#define CELLS(row,col) cells[row*cols+col]

static void ShowWarning
    (
    const char*  formatSpec,
    ...
    )
{
    va_list               ap;
    va_start (ap, formatSpec);
    (void) vfprintf (stderr, formatSpec, ap);
    fprintf (stderr, "\n");
    va_end (ap);
}

typedef struct RecordNode_s {
    int                   index;
    int                   page_id;
    boolean               done;
    struct RecordNode_s*  next;
} RecordNode;

RecordNode*  records = NULL;

static int GetNextRecordNumber ()
{
    RecordNode*  ptr;
    int          i = 0;

    for (ptr = records; ptr != NULL; ptr = ptr->next) {
        if (!ptr->done) {
            i = ptr->index;
            break;
        }
    }
    return i;
}

static void LinkRecords
    (
    char*  dir
    )
{
    RecordNode*  ptr;
    char*        realfilename;
    char*        linkname;

    realfilename = malloc (strlen (dir) + 20);
    linkname = malloc (strlen (dir) + 20);

    for (ptr = records; ptr != NULL; ptr = ptr->next) {
        if (ptr->page_id != ptr->index) {
            sprintf (realfilename, "%s/r%d.html", dir, ptr->page_id);
            sprintf (linkname, "%s/r%d.html", dir, ptr->index);
            link (realfilename, linkname);
        }
    }

    free (realfilename);
    free (linkname);
}

static int GetPageID
    (
    int  record_index
    )
{
    RecordNode*  ptr;

    for (ptr = records; ptr != NULL; ptr = ptr->next) {
        if (ptr->index == record_index)
            return ptr->page_id;
    }
    return 0;
}

static void AddRecord
    (
    int  index
    )
{
    RecordNode*  newr;
    RecordNode*  ptr;

    for (ptr = records; ptr != NULL; ptr = ptr->next) {
        if (ptr->index == index)
            return;
    }
    newr = (RecordNode *) malloc (sizeof (RecordNode));
    newr->done = FALSE;
    newr->index = index;
    newr->page_id = index;
    newr->next = records;
    records = newr;
}

static void MarkRecordDone
    (
    int  index
    )
{
    RecordNode*  ptr;
    for (ptr = records; ptr != NULL; ptr = ptr->next) {
        if (ptr->index == index) {
            ptr->done = TRUE;
            return;
        }
    }
    if (ptr == NULL) {
        AddRecord (index);
        MarkRecordDone (index);
    }
}

static void SetPageID
    (
    int  index,
    int  page_id
    )
{
    RecordNode*  ptr;

    for (ptr = records; ptr != NULL; ptr = ptr->next) {
        if (ptr->index == index) {
            ptr->page_id = page_id;
            return;
        }
    }
    if (ptr == NULL) {
        AddRecord (index);
        SetPageID (index, page_id);
    }
}

static char* MailtoURLFromBytes
    (
    unsigned char*  record_data,
    int             len
    )
{
    int    to_offset;
    int    cc_offset;
    int    subject_offset;
    int    body_offset;
    int    url_size;
    char*  url;
    char*  bytes;

    bytes = record_data + 8;

    to_offset = (bytes[0] << 8) + bytes[1];
    cc_offset = (bytes[2] << 8) + bytes[3];
    subject_offset = (bytes[4] << 8) + bytes[5];
    body_offset = (bytes[6] << 8) + bytes[7];
    url_size = strlen ("mailto:") + 2;
    if (to_offset != 0) {
        url_size += strlen ((char *) (bytes + to_offset));
    }
    if (cc_offset != 0) {
        url_size +=
            (strlen ((char *) (bytes + cc_offset)) + strlen ("&cc="));
    }
    if (subject_offset != 0) {
        url_size +=
            (strlen ((char *) (bytes + subject_offset)) +
             strlen ("&subject="));
    }
    if (body_offset != 0) {
        url_size +=
            (strlen ((char *) (bytes + body_offset)) + strlen ("&body="));
    }
    url = (char *) malloc (url_size);
    strcpy (url, "mailto:");
    if (to_offset != 0) {
        strcpy (url + strlen (url), ((char *) (bytes + to_offset)));
    }
    if ((cc_offset != 0) || (subject_offset != 0) || (body_offset != 0)) {
        strcpy (url + strlen (url), "?");
    }
    if (cc_offset != 0) {
        strcpy (url + strlen (url), "cc=");
        strcpy (url + strlen (url), (char *) (bytes + cc_offset));
    }
    if (subject_offset != 0) {
        strcpy (url + strlen (url), "subject=");
        strcpy (url + strlen (url), (char *) (bytes + subject_offset));
    }
    if (body_offset != 0) {
        strcpy (url + strlen (url), "body=");
        strcpy (url + strlen (url), (char *) (bytes + body_offset));
    }
    return url;
}

/***********************************************************************/
/***********************************************************************/
/*****                                                             *****/
/*****   Code to decode the Palm image format to JPEG              *****/
/*****                                                             *****/
/***********************************************************************/
/***********************************************************************/

#define READ_BIGENDIAN_SHORT(p) (((p)[0] << 8)|((p)[1]))
#define READ_BIGENDIAN_LONG(p)  (((p)[0] << 24)|((p)[1] << 16)|((p)[2] << 8)|((p)[3]))

#define PALM_IS_COMPRESSED_FLAG       0x8000
#define PALM_HAS_COLORMAP_FLAG        0x4000
#define PALM_HAS_TRANSPARENCY_FLAG    0x2000
#define PALM_DIRECT_COLOR_FLAG        0x0400
#define PALM_4_BYTE_FIELD_FLAG        0x0200

#define PALM_COMPRESSION_SCANLINE     0x00
#define PALM_COMPRESSION_RLE          0x01
#define PALM_COMPRESSION_PACKBITS     0x02
#define PALM_COMPRESSION_NONE         0xFF

#define PALM_COLORMAP_SIZE            232

typedef struct {
    unsigned char         red;
    unsigned char         green;
    unsigned char         blue;
} ColorMapEntry;

static ColorMapEntry Palm8BitColormap[] = {
    {255, 255, 255}, {255, 204, 255}, {255, 153, 255}, {255, 102, 255},
    {255,  51, 255}, {255,   0, 255}, {255, 255, 204}, {255, 204, 204},
    {255, 153, 204}, {255, 102, 204}, {255,  51, 204}, {255,   0, 204},
    {255, 255, 153}, {255, 204, 153}, {255, 153, 153}, {255, 102, 153},
    {255,  51, 153}, {255,   0, 153}, {204, 255, 255}, {204, 204, 255},
    {204, 153, 255}, {204, 102, 255}, {204,  51, 255}, {204,   0, 255},
    {204, 255, 204}, {204, 204, 204}, {204, 153, 204}, {204, 102, 204},
    {204,  51, 204}, {204,   0, 204}, {204, 255, 153}, {204, 204, 153},
    {204, 153, 153}, {204, 102, 153}, {204,  51, 153}, {204,   0, 153},
    {153, 255, 255}, {153, 204, 255}, {153, 153, 255}, {153, 102, 255},
    {153,  51, 255}, {153,   0, 255}, {153, 255, 204}, {153, 204, 204},
    {153, 153, 204}, {153, 102, 204}, {153,  51, 204}, {153,   0, 204},
    {153, 255, 153}, {153, 204, 153}, {153, 153, 153}, {153, 102, 153},
    {153,  51, 153}, {153,   0, 153}, {102, 255, 255}, {102, 204, 255},
    {102, 153, 255}, {102, 102, 255}, {102,  51, 255}, {102,   0, 255},
    {102, 255, 204}, {102, 204, 204}, {102, 153, 204}, {102, 102, 204},
    {102,  51, 204}, {102,   0, 204}, {102, 255, 153}, {102, 204, 153},
    {102, 153, 153}, {102, 102, 153}, {102,  51, 153}, {102,   0, 153},
    { 51, 255, 255}, { 51, 204, 255}, { 51, 153, 255}, { 51, 102, 255},
    { 51,  51, 255}, { 51,   0, 255}, { 51, 255, 204}, { 51, 204, 204},
    { 51, 153, 204}, { 51, 102, 204}, { 51,  51, 204}, { 51,   0, 204},
    { 51, 255, 153}, { 51, 204, 153}, { 51, 153, 153}, { 51, 102, 153},
    { 51,  51, 153}, { 51,   0, 153}, {  0, 255, 255}, {  0, 204, 255},
    {  0, 153, 255}, {  0, 102, 255}, {  0,  51, 255}, {  0,   0, 255},
    {  0, 255, 204}, {  0, 204, 204}, {  0, 153, 204}, {  0, 102, 204},
    {  0,  51, 204}, {  0,   0, 204}, {  0, 255, 153}, {  0, 204, 153},
    {  0, 153, 153}, {  0, 102, 153}, {  0,  51, 153}, {  0,   0, 153},
    {255, 255, 102}, {255, 204, 102}, {255, 153, 102}, {255, 102, 102},
    {255,  51, 102}, {255,   0, 102}, {255, 255,  51}, {255, 204,  51},
    {255, 153,  51}, {255, 102,  51}, {255,  51,  51}, {255,   0,  51},
    {255, 255,   0}, {255, 204,   0}, {255, 153,   0}, {255, 102,   0},
    {255,  51,   0}, {255,   0,   0}, {204, 255, 102}, {204, 204, 102},
    {204, 153, 102}, {204, 102, 102}, {204,  51, 102}, {204,   0, 102},
    {204, 255,  51}, {204, 204,  51}, {204, 153,  51}, {204, 102,  51},
    {204,  51,  51}, {204,   0,  51}, {204, 255,   0}, {204, 204,   0},
    {204, 153,   0}, {204, 102,   0}, {204,  51,   0}, {204,   0,   0},
    {153, 255, 102}, {153, 204, 102}, {153, 153, 102}, {153, 102, 102},
    {153,  51, 102}, {153,   0, 102}, {153, 255,  51}, {153, 204,  51},
    {153, 153,  51}, {153, 102,  51}, {153,  51,  51}, {153,   0,  51},
    {153, 255,   0}, {153, 204,   0}, {153, 153,   0}, {153, 102,   0},
    {153,  51,   0}, {153,   0,   0}, {102, 255, 102}, {102, 204, 102},
    {102, 153, 102}, {102, 102, 102}, {102,  51, 102}, {102,   0, 102},
    {102, 255,  51}, {102, 204,  51}, {102, 153,  51}, {102, 102,  51},
    {102,  51,  51}, {102,   0,  51}, {102, 255,   0}, {102, 204,   0},
    {102, 153,   0}, {102, 102,   0}, {102,  51,   0}, {102,   0,   0},
    { 51, 255, 102}, { 51, 204, 102}, { 51, 153, 102}, { 51, 102, 102},
    { 51,  51, 102}, { 51,   0, 102}, { 51, 255,  51}, { 51, 204,  51},
    { 51, 153,  51}, { 51, 102,  51}, { 51,  51,  51}, { 51,   0,  51},
    { 51, 255,   0}, { 51, 204,   0}, { 51, 153,   0}, { 51, 102,   0},
    { 51,  51,   0}, { 51,   0,   0}, {  0, 255, 102}, {  0, 204, 102},
    {  0, 153, 102}, {  0, 102, 102}, {  0,  51, 102}, {  0,   0, 102},
    {  0, 255,  51}, {  0, 204,  51}, {  0, 153,  51}, {  0, 102,  51},
    {  0,  51,  51}, {  0,   0,  51}, {  0, 255,   0}, {  0, 204,   0},
    {  0, 153,   0}, {  0, 102,   0}, {  0,  51,   0}, { 17,  17,  17},
    { 34,  34,  34}, { 68,  68,  68}, { 85,  85,  85}, {119, 119, 119},
    {136, 136, 136}, {170, 170, 170}, {187, 187, 187}, {221, 221, 221},
    {238, 238, 238}, {192, 192, 192}, {128,   0,   0}, {128,   0, 128},
    {  0, 128,   0}, {  0, 128, 128}, {  0,   0,   0}, {  0,   0,   0},
    {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}
};

static ColorMapEntry Palm1BitColormap[] = {
    {255, 255, 255}, {  0,   0,   0} };

static ColorMapEntry Palm2BitColormap[] = {
    {255, 255, 255}, {192, 192, 192}, {128, 128, 128}, {  0,   0,   0}
};

static ColorMapEntry Palm4BitColormap[] = {
    {255, 255, 255}, {238, 238, 238}, {221, 221, 221}, {204, 204, 204},
    {187, 187, 187}, {170, 170, 170}, {153, 153, 153}, {136, 136, 136},
    {119, 119, 119}, {102, 102, 102}, { 85,  85,  85}, { 68,  68,  68},
    { 51,  51,  51}, { 34,  34,  34}, { 17,  17,  17}, {  0,   0,   0}
};

static boolean TranscribePalmImageToJPEG
    (
    unsigned char *image_bytes_in,
    int byte_count_in,
    FILE * outfile
    )
{
    unsigned int    width;
    unsigned int    height;
    unsigned int    bytes_per_row;
    unsigned int    flags;
    unsigned int    next_depth_offset;
    unsigned int    bits_per_pixel;
    unsigned int    version;
    unsigned int    transparent_index;
    unsigned int    compression_type;
    unsigned int    i;
    unsigned int    j;
    unsigned int    inval;
    unsigned int    inbit;
    unsigned int    mask;
    unsigned int    incount;
    unsigned int    palm_red_bits;
    unsigned int    palm_green_bits;
    unsigned int    palm_blue_bits;
    unsigned char*  palm_ptr;
    unsigned char*  x_ptr;
    unsigned char*  imagedata;
    unsigned char*  inbyte;
    unsigned char*  rowbuf;
    unsigned char*  lastrow;
    unsigned char*  imagedatastart;
    unsigned char*  palmimage;
    ColorMapEntry  *colormap;

    JSAMPLE*                     jpeg_row;
    struct jpeg_compress_struct  cinfo;
    struct jpeg_error_mgr        jerr;
    JSAMPROW                     row_pointer[1];/* pointer to JSAMPLE row[s] */

    palmimage = image_bytes_in;
    width = READ_BIGENDIAN_SHORT (palmimage + 0);
    height = READ_BIGENDIAN_SHORT (palmimage + 2);
    bytes_per_row = READ_BIGENDIAN_SHORT (palmimage + 4);
    flags = READ_BIGENDIAN_SHORT (palmimage + 6);
    bits_per_pixel = palmimage[8];
    version = palmimage[9];
    next_depth_offset = READ_BIGENDIAN_SHORT (palmimage + 10);
    transparent_index = palmimage[12];
    compression_type = palmimage[13];
    /* bytes 14 and 15 are reserved by Palm and always 0 */

#if 0
    ShowWarning
        ("Palm image is %dx%d, %d bpp, version %d, flags 0x%x, compression %d",
         width, height, bits_per_pixel, version, flags, compression_type);
#endif

    if (compression_type == PALM_COMPRESSION_PACKBITS) {
        ShowWarning
            ("Image uses packbits compression; not yet supported");
        return FALSE;
    }
    else if ((compression_type != PALM_COMPRESSION_NONE) &&
             (compression_type != PALM_COMPRESSION_RLE) &&
             (compression_type != PALM_COMPRESSION_SCANLINE)) {
        ShowWarning ("Image uses unknown compression, code 0x%x",
                      compression_type);
        return FALSE;
    }

    /* as of PalmOS 4.0, there are 6 different kinds of Palm pixmaps:

       1, 2, or 4 bit grayscale
       8-bit StaticColor using the Palm standard colormap
       8-bit PseudoColor using a user-specified colormap
       16-bit DirectColor using 5 bits for red, 6 for green, and 5 for blue

       Each of these can be compressed with one of four compression schemes,
       "RLE", "Scanline", "PackBits", or none.

       We begin by constructing the colormap.
     */

    if (flags & PALM_HAS_COLORMAP_FLAG) {
        ShowWarning
            ("Palm images with custom colormaps are not currently supported.\n");
        return FALSE;
    }
    else if (bits_per_pixel == 1) {
        colormap = Palm1BitColormap;
        imagedatastart = palmimage + 16;
    }
    else if (bits_per_pixel == 2) {
        colormap = Palm2BitColormap;
        imagedatastart = palmimage + 16;
    }
    else if (bits_per_pixel == 4) {
        colormap = Palm4BitColormap;
        imagedatastart = palmimage + 16;
    }
    else if (bits_per_pixel == 8) {
        colormap = Palm8BitColormap;
        imagedatastart = palmimage + 16;
    }
    else if (bits_per_pixel == 16 && (flags & PALM_DIRECT_COLOR_FLAG)) {
        colormap = NULL;
        palm_red_bits = palmimage[16];
        palm_green_bits = palmimage[17];
        palm_blue_bits = palmimage[18];
        if (palm_blue_bits > 8 || palm_green_bits > 8 || palm_red_bits > 8) {
            ShowWarning
                ("Can't handle this format DirectColor image -- too wide in some color (%d:%d:%d)\n",
                 palm_red_bits, palm_green_bits, palm_blue_bits);
            return FALSE;
        }
        if (bits_per_pixel > (8 * sizeof (unsigned long))) {
            ShowWarning
                ("Can't handle this format DirectColor image -- too many bits per pixel (%d)\n",
                 bits_per_pixel);
            return FALSE;
        }
        imagedatastart = palmimage + 24;
    }
    else {
        ShowWarning ("Unknown bits-per-pixel of %d encountered.\n",
                      bits_per_pixel);
        return FALSE;
    }

    /* now create the JPEG image row buffer */
    jpeg_row = (JSAMPLE *) malloc (sizeof (JSAMPLE) * (width * 3));

    /* Use standard JPEG error processing */
    cinfo.err = jpeg_std_error (&jerr);
    /* Initialize the JPEG compression object. */
    jpeg_create_compress (&cinfo);

    jpeg_stdio_dest (&cinfo, outfile);

    cinfo.image_width = width;  /* image width and height, in pixels */
    cinfo.image_height = height;
    cinfo.input_components = 3; /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;     /* colorspace of input image */

    jpeg_set_defaults (&cinfo);
    jpeg_set_quality (&cinfo, 100,
                      TRUE /* limit to baseline-JPEG values */ );

    row_pointer[0] = &jpeg_row[0];

    jpeg_start_compress (&cinfo, TRUE);

    /* row by row, uncompress the Palm image and copy it to the JPEG buffer */
    rowbuf = (unsigned char *) malloc (bytes_per_row * width);
    lastrow = (unsigned char *) malloc (bytes_per_row * width);
    for (i = 0, palm_ptr = imagedatastart, x_ptr = imagedata; i < height;
         ++i) {

        /* first, uncompress the Palm image */
        if ((flags & PALM_IS_COMPRESSED_FLAG)
            && (compression_type == PALM_COMPRESSION_RLE)) {
            for (j = 0; j < bytes_per_row;) {
                incount = *palm_ptr++;
                inval = *palm_ptr++;
                memset (rowbuf + j, inval, incount);
                j += incount;
            }
        }
        else if ((flags & PALM_IS_COMPRESSED_FLAG)
                 && (compression_type == PALM_COMPRESSION_SCANLINE)) {
            for (j = 0; j < bytes_per_row; j += 8) {
                incount = *palm_ptr++;
                inval =
                    ((bytes_per_row - j) < 8) ? (bytes_per_row - j) : 8;
                for (inbit = 0; inbit < inval; inbit += 1) {
                    if (incount & (1 << (7 - inbit)))
                        rowbuf[j + inbit] = *palm_ptr++;
                    else
                        rowbuf[j + inbit] = lastrow[j + inbit];
                }
            }
            memcpy (lastrow, rowbuf, bytes_per_row);
        }
        else if (((flags & PALM_IS_COMPRESSED_FLAG) &&
                  (compression_type == PALM_COMPRESSION_NONE)) ||
                 (flags && PALM_IS_COMPRESSED_FLAG) == 0) {
            memcpy (rowbuf, palm_ptr, bytes_per_row);
            palm_ptr += bytes_per_row;
        }

        /* next, write it to the GDK bitmap */
        if (colormap) {
            mask = (1 << bits_per_pixel) - 1;
            for (inbit = 8 - bits_per_pixel, inbyte = rowbuf, j = 0;
                 j < width; ++j) {
                inval = ((*inbyte) & (mask << inbit)) >> inbit;
                /* correct for oddity of the 8-bit color Palm pixmap... */
                if ((bits_per_pixel == 8) && (inval == 0xFF))
                    inval = 231;
                /* now lookup the correct color and set the pixel in the GTK bitmap */
                jpeg_row[(j * 3) + 0] = colormap[inval].red;
                jpeg_row[(j * 3) + 1] = colormap[inval].green;
                jpeg_row[(j * 3) + 2] = colormap[inval].blue;
                if (!inbit) {
                    ++inbyte;
                    inbit = 8 - bits_per_pixel;
                }
                else {
                    inbit -= bits_per_pixel;
                }
            }
        }
        else if (!colormap && bits_per_pixel == 16) {
            for (inbyte = rowbuf, j = 0; j < width; ++j) {
                inval = (inbyte[0] << 8) | inbyte[1];
#if 0
                ShowWarning ("pixel is %d,%d (%02x:%02x:%02x)",
                              j, i,
                              (inval >> (bits_per_pixel - palm_red_bits)) &
                              ((1 << palm_red_bits) - 1),
                              (inval >> palm_blue_bits) &
                              ((1 << palm_green_bits) - 1),
                              (inval >> 0) & ((1 << palm_blue_bits) - 1));
#endif
                jpeg_row[(j * 3) + 0] =
                    (inval >> (bits_per_pixel - palm_red_bits)) &
                    ((1 << palm_red_bits) - 1);
                jpeg_row[(j * 3) + 1] =
                    (inval >> palm_blue_bits) & ((1 << palm_green_bits) -
                                                 1);
                jpeg_row[(j * 3) + 2] =
                    (inval >> 0) & ((1 << palm_blue_bits) - 1);
                inbyte += 2;
            }
        }

        (void) jpeg_write_scanlines (&cinfo, row_pointer, 1);
    }

    free (rowbuf);
    free (lastrow);
    free (jpeg_row);

    jpeg_finish_compress (&cinfo);
    jpeg_destroy_compress (&cinfo);

    return TRUE;
}

static boolean TranscribeImageRecord
    (
    plkr_Document*       doc,
    int                  id,
    FILE*                fp,
    unsigned char*       bytes,
    int                  len,
    plkr_DataRecordType  type
    )
{
    return TranscribePalmImageToJPEG (bytes + 8, len - 8, fp);
}

typedef struct {
    unsigned int    width;
    unsigned int    height;
    unsigned int    bytes_per_row;
    unsigned int    flags;
    unsigned int    next_depth_offset;
    unsigned int    bits_per_pixel;
    unsigned int    version;
    unsigned int    transparent_index;
    unsigned int    compression_type;
    unsigned int    palm_red_bits;
    unsigned int    palm_green_bits;
    unsigned int    palm_blue_bits;
    unsigned char*  bytes;

} PALMPIX;

static boolean TranscribeMultiImageRecord
    (
    plkr_Document*       doc,
    int                  id,
    FILE*                fp,
    unsigned char*       bytes,
    int                  len,
    plkr_DataRecordType  type
    )
{
    unsigned char*       pbytes;
    unsigned char*       outbytes;
    unsigned char*       outptr;
    unsigned char*       ptr = &bytes[12];
    plkr_DataRecordType  ptype;
    PALMPIX*             cells;
    PALMPIX*             acell;
    unsigned int         record_id;
    unsigned int         plen;
    unsigned int         x;
    unsigned int         y;
    unsigned int         cols;
    unsigned int         rows;
    unsigned int         width;
    unsigned int         height;
    unsigned int         bytes_per_row;
    unsigned int         flags;
    unsigned int         bits_per_pixel;
    unsigned int         version;
    unsigned int         transparent_index;
    unsigned int         compression_type;
    unsigned int         palm_red_bits;
    unsigned int         palm_green_bits;
    unsigned int         palm_blue_bits;
    unsigned int         outlen;
    unsigned int         offset;
    boolean              status;

    cols = (bytes[8] << 8) + bytes[9];
    rows = (bytes[10] << 8) + bytes[11];

    cells = (PALMPIX *) calloc (cols * rows, sizeof (PALMPIX));

    height = 0;
    for (y = 0; y < rows; y++) {
        width = 0;
        bytes_per_row = 0;
        for (x = 0; x < cols; x++) {
            acell = &CELLS (y, x);
            record_id = (ptr[0] << 8) + ptr[1];
            ptr += 2;
            pbytes = plkr_GetRecordBytes (doc, record_id, &plen, &ptype);
            if (pbytes == NULL) {
                free (cells);
                return FALSE;
            }

            pbytes += 8;
            acell->width = READ_BIGENDIAN_SHORT (&pbytes[0]);
            width += acell->width;
            acell->height = READ_BIGENDIAN_SHORT (&pbytes[2]);
            acell->bytes_per_row = READ_BIGENDIAN_SHORT (&pbytes[4]);
            bytes_per_row += acell->bytes_per_row;
            acell->flags = READ_BIGENDIAN_SHORT (&pbytes[6]);
            flags = acell->flags;
            acell->bits_per_pixel = pbytes[8];
            bits_per_pixel = acell->bits_per_pixel;
            acell->version = pbytes[9];
            version = acell->version;
            acell->next_depth_offset = READ_BIGENDIAN_SHORT (&pbytes[10]);
            acell->transparent_index = pbytes[12];
            transparent_index = acell->transparent_index;
            acell->compression_type = pbytes[13];
            compression_type = acell->compression_type;

            if (acell->flags & PALM_HAS_COLORMAP_FLAG) {
                ShowWarning
                    ("Palm images with custom colormaps are not currently supported.\n");
                free (cells);
                return FALSE;
            }

            acell->bytes = pbytes + 16;
            offset = 16;
            if (acell->bits_per_pixel == 16
                && (acell->flags & PALM_DIRECT_COLOR_FLAG)) {
                acell->palm_red_bits = pbytes[16];
                palm_red_bits = acell->palm_red_bits;
                acell->palm_green_bits = pbytes[17];
                palm_green_bits = acell->palm_green_bits;
                acell->palm_blue_bits = pbytes[18];
                palm_blue_bits = acell->palm_blue_bits;
                acell->bytes = pbytes + 24;
                offset = 24;
            }
        }
        height += acell->height;
    }

    outlen = bytes_per_row * height + offset;
    outbytes = (unsigned char *) malloc (outlen);
    outptr = outbytes;

    *outptr++ = width >> 8;
    *outptr++ = width;
    *outptr++ = height >> 8;
    *outptr++ = height;
    *outptr++ = bytes_per_row >> 8;
    *outptr++ = bytes_per_row;
    *outptr++ = flags >> 8;
    *outptr++ = flags;
    *outptr++ = bits_per_pixel;
    *outptr++ = version;
    *outptr++ = 0;              /* next_depth_offset */
    *outptr++ = 0;
    *outptr++ = transparent_index;
    *outptr++ = compression_type;
    *outptr++ = 0;
    *outptr++ = 0;

    if (acell->bits_per_pixel == 16
        && (acell->flags & PALM_DIRECT_COLOR_FLAG)) {
        *outptr++ = palm_red_bits;
        *outptr++ = palm_green_bits;
        *outptr++ = palm_blue_bits;
        *outptr++ = 0;
        *outptr++ = 0;
        *outptr++ = 0;
        *outptr++ = 0;
        *outptr++ = 0;
    }

    for (y = 0; y < rows; y++) {
        int                   i, h;
        acell = &CELLS (y, 0);
        h = acell->height;
        for (i = 0; i < h; i++) {
            for (x = 0; x < cols; x++) {
                acell = &CELLS (y, x);
                memcpy (outptr, acell->bytes, acell->bytes_per_row);
                acell->bytes += acell->bytes_per_row;
                outptr += acell->bytes_per_row;
            }
        }
    }

    status = TranscribePalmImageToJPEG (outbytes, outlen, fp);

    free (outbytes);
    free (cells);

    return status;
}

static void DoStyle
    (
    FILE*    fp,
    int      style,
    boolean  start
    )
{
    char  end[2];

    if (start)
        strcpy (end, "");
    else
        strcpy (end, "/");

    switch (style) {
        case 1:
            fprintf (fp, "<%sH1>", end);
            break;
        case 2:
            fprintf (fp, "<%sH2>", end);
            break;
        case 3:
            fprintf (fp, "<%sH3>", end);
            break;
        case 4:
            fprintf (fp, "<%sH4>", end);
            break;
        case 5:
            fprintf (fp, "<%sH5>", end);
            break;
        case 6:
            fprintf (fp, "<%sH6>", end);
            break;
        case 7:
            fprintf (fp, "<%sB>", end);
            break;
        case 8:
            fprintf (fp, "<%sTT>", end);
            break;
    }
}

static boolean TranscribeTableRecord
    (
    plkr_Document*       doc,
    int                  id,
    FILE*                fp,
    unsigned char*       bytes,
    int                  len,
    plkr_DataRecordType  type
    );

static void ParseText
    (
    plkr_Document*  doc,
    unsigned char*  ptr,
    int             text_len,
    int*            font,
    long*           text_color,
    int*            style,
    FILE*           fp
    )
{
    unsigned char*  end;
    int             fctype;
    int             fclen;

    end = ptr + text_len;
    while (ptr < end) {
        if (ptr[0]) {
            fprintf (fp, "%s", ptr);
            ptr += strlen (ptr);
        }
        else {
            fctype = GET_FUNCTION_CODE_TYPE (ptr[1]);
            fclen = 2 + GET_FUNCTION_CODE_DATALEN (ptr[1]);
            switch (fctype) {
                case PLKR_TFC_LINK:
                    switch (fclen) {
                        case 4:        /* ANCHOR_BEGIN */
                            {
                                int                   record_id =
                                    (ptr[2] << 8) + ptr[3];
                                plkr_DataRecordType   type =
                                    plkr_GetRecordType (doc, record_id);

                                if (type ==
                                    PLKR_DRTYPE_IMAGE
                                    || type ==
                                    PLKR_DRTYPE_IMAGE_COMPRESSED)
                                    fprintf (fp,
                                             "<A HREF=\"r%d.jpg\">",
                                             record_id);
                                else
                                    fprintf (fp,
                                             "<A HREF=\"r%d.html\">",
                                             record_id);
                                AddRecord (record_id);
                            }
                            break;
                        case 2:        /* ANCHOR_END */
                            fprintf (fp, "</A>");
                            break;
                    }
                    ptr += fclen;
                    break;
                case PLKR_TFC_FONT:
                    DoStyle (fp, *style, FALSE);
                    *style = ptr[2];
                    DoStyle (fp, *style, TRUE);
                    ptr += fclen;
                    break;
                case PLKR_TFC_NEWLINE:
                    fprintf (fp, "<BR>\n");
                    ptr += fclen;
                    break;
                case PLKR_TFC_BITALIC:
                    fprintf (fp, "<I>");
                    ptr += fclen;
                    break;
                case PLKR_TFC_EITALIC:
                    fprintf (fp, "</I>");
                    ptr += fclen;
                    break;
                case PLKR_TFC_COLOR:
                    if (*font) {
                        *font--;
                        fprintf (fp, "</FONT>");
                    }
                    *text_color = (ptr[2] << 16) + (ptr[3] << 8) + ptr[4];
                    fprintf (fp, "<FONT color=\"#%06X\">", *text_color);
                    *font++;
                    ptr += fclen;
                    break;
                case PLKR_TFC_BULINE:
                    fprintf (fp, "<U>");
                    ptr += fclen;
                    break;
                case PLKR_TFC_EULINE:
                    fprintf (fp, "</U>");
                    ptr += fclen;
                    break;
                case PLKR_TFC_BSTRIKE:
                    fprintf (fp, "<S>");
                    ptr += fclen;
                    break;
                case PLKR_TFC_ESTRIKE:
                    fprintf (fp, "</S>");
                    ptr += fclen;
                    break;
                case PLKR_TFC_TABLE:
                    if (fclen == 4) {
                        int                   record_id, datalen;
                        plkr_DataRecordType   type = 0;
                        unsigned char        *bytes = NULL;

                        record_id = (ptr[2] << 8) + ptr[3];
                        bytes =
                            plkr_GetRecordBytes
                            (doc, record_id, &datalen, &type);
                        TranscribeTableRecord (doc,
                                               record_id,
                                               fp, bytes, datalen, type);
                    }
                    ptr += fclen;
                    break;
                default:
                    ptr += fclen;
            }
        }
    }
}


static boolean TranscribeTableRecord
    (
    plkr_Document*       doc,
    int                  id,
    FILE*                fp,
    unsigned char*       bytes,
    int                  len,
    plkr_DataRecordType  type
    )
{
    unsigned char*  ptr = &bytes[24];
    unsigned char*  end;
    unsigned char*  text_end;
    char*           align_names[] = { "left", "right", "center" };
    boolean         in_row = FALSE;
    int             x;
    int             y;
    int             cols;
    int             size;
    int             rows;
    int             border;
    int             record_id;
    int             align;
    int             text_len;
    int             colspan;
    int             rowspan;
    int             font = 0;
    int             style = 0;
    int             fctype;
    int             fclen;
    long            border_color;
    long            link_color;
    long            text_color = 0;

    size = (bytes[8] << 8) + bytes[9];
    cols = (bytes[10] << 8) + bytes[11];
    rows = (bytes[12] << 8) + bytes[13];
    border = bytes[15];
    border_color = (bytes[17] << 16) + (bytes[18] << 8) + (bytes[19] << 8);
    link_color = (bytes[21] << 16) + (bytes[22] << 8) + (bytes[23] << 8);

    end = ptr + size - 1;

    fprintf (fp, "<TABLE border=%d bordercolor=\"#%06X\" "
             "linkcolor=\"#%06X\">\n", border, border_color, link_color);

    while (ptr < end) {
        if (ptr[0] == '\0') {
            fctype = GET_FUNCTION_CODE_TYPE (ptr[1]);
            fclen = 2 + GET_FUNCTION_CODE_DATALEN (ptr[1]);
            switch (fctype) {
                case PLKR_TFC_TABLE:
                    switch (fclen) {
                        case 2:        /* NEW_ROW */
                            if (in_row)
                                fprintf (fp, "</TR>\n");
                            fprintf (fp, "<TR>\n");
                            in_row = TRUE;
                            ptr += fclen;
                            break;
                        case 9:        /* NEW_CELL */
                            align = ptr[2];
                            colspan = ptr[5];
                            rowspan = ptr[6];
                            fprintf (fp, "<TD align=\"%s\" colspan=%d "
                                     "rowspan=%d bordercolor=\"#%06X\">",
                                     align_names[align], colspan, rowspan,
                                     border_color);

                            if (record_id = READ_BIGENDIAN_SHORT (&ptr[3])) {
                                fprintf (fp, "<IMG SRC=\"r%d.jpg\">",
                                         record_id);
                                AddRecord (record_id);
                            }
                            DoStyle (fp, style, TRUE);
                            text_len = READ_BIGENDIAN_SHORT (&ptr[7]);
                            ptr += fclen;
                            ParseText (doc, ptr, text_len, &font,
                                        &text_color, &style, fp);
                            ptr += text_len;
                            DoStyle (fp, style, FALSE);
                            fprintf (fp, "</TD>\n");
                            break;
                        default:
                            ptr += fclen;
                    }
                    break;
                default:
                    ptr += fclen;
            }
        }
        else {
            fprintf (fp, "</TABLE>\n");
            return FALSE;
        }
    }

    fprintf (fp, "</TABLE>\n");
    return TRUE;
}

typedef struct {
    int   size;
    int   attributes;
} ParagraphInfo;

static ParagraphInfo *ParseParagraphInfo
    (
    unsigned char*  bytes,
    int             len,
    int*            nparas
    )
{
    ParagraphInfo*  paragraph_info;
    int             j;
    int             n;

    n = (bytes[2] << 8) + bytes[3];
    paragraph_info = (ParagraphInfo *) malloc (sizeof (ParagraphInfo) * n);
    for (j = 0; j < n; j++) {
        paragraph_info[j].size =
            (bytes[8 + (j * 4) + 0] << 8) + bytes[8 + (j * 4) + 1];
        paragraph_info[j].attributes =
            (bytes[8 + (j * 4) + 2] << 8) + bytes[8 + (j * 4) + 3];
    }
    *nparas = n;
    return paragraph_info;
}

static boolean TranscribeTextRecord
    (
    plkr_Document*       doc,
    int                  id,
    FILE*                fp,
    unsigned char*       bytes,
    int                  len,
    plkr_DataRecordType  type
    )
{
    unsigned char*  ptr;
    unsigned char*  run;
    unsigned char*  para_start;
    unsigned char*  data;
    unsigned char*  start;
    ParagraphInfo*  paragraphs;
    boolean         first_record_of_page = TRUE;
    boolean         current_link;
    boolean         current_italic;
    boolean         current_struckthrough;
    boolean         current_underline;
    int             home_id = plkr_GetHomeRecordID (doc);
    int             fctype;
    int             fclen;
    int             para_index;
    int             para_len;
    int             textlen;
    int             data_len;
    int             current_font;
    int             record_index;
    int             current_alignment;
    int             current_left_margin;
    int             current_right_margin;
    int             nparagraphs;
    long            current_color;

    record_index = id;

    fprintf (fp, "<HTML><HEAD>\n<TITLE>%s", plkr_GetName (doc));
    if (id != home_id)
        fprintf (fp, ":  %d\n", id);
    fprintf (fp, "</TITLE>\n");
    if (home_id == id) {
        char                 *url = plkr_GetRecordURL (doc, id);
        if (url)
            fprintf (fp, "<!-- original URL %s -->\n", url);
    }
    fprintf (fp, "</HEAD>\n<BODY>\n");

    paragraphs = ParseParagraphInfo (bytes, len, &nparagraphs);
    start = bytes + 8 + ((bytes[2] << 8) + bytes[3]) * 4;

    for (para_index = 0, ptr = start, run = start;
         para_index < nparagraphs; para_index++) {

        para_len = paragraphs[para_index].size;

        /* If the paragraph is the last in the record, and it consists
           of a link to the next record in the logical page, we trim off
           the paragraph and instead insert the whole page */

        if (((para_index + 1) == nparagraphs) &&
            (para_len == (sizeof ("Click here for the next part") + 5)) &&
            (*ptr == 0) && (ptr[1] == ((PLKR_TFC_LINK << 3) + 2)) &&
            (strcmp (ptr + 4, "Click here for the next part") == 0)) {

            record_index = (ptr[2] << 8) + ptr[3];
            if ((data =
                 plkr_GetRecordBytes (doc, record_index, &data_len,
                                      &type)) == NULL) {
                ShowWarning ("Can't open record %d!", record_index);
                return FALSE;
            }
            else if (!(type == PLKR_DRTYPE_TEXT_COMPRESSED ||
                       type == PLKR_DRTYPE_TEXT)) {
                ShowWarning
                    ("Bad record type %d in record linked from end of record %d",
                     type, id);
                return FALSE;
            }
            first_record_of_page = FALSE;
            para_index = 0;
            ptr = data + 8 + ((data[2] << 8) + data[3]) * 4;
            run = ptr;
            free (paragraphs);
            paragraphs = ParseParagraphInfo (data, data_len, &nparagraphs);
            para_len = paragraphs[para_index].size;
            MarkRecordDone (record_index);
            SetPageID (record_index, id);
        }

        if ((para_index == 0) && !first_record_of_page &&
            (*ptr == 0) && (ptr[1] == ((PLKR_TFC_LINK << 3) + 2)) &&
            (strcmp (ptr + 4, "Click here for the previous part") == 0)) {
            /* throw away this inserted paragraph */
            ptr += para_len;
            run = ptr;
            continue;
        }

        fprintf (fp,
                 "<p style=\"margin-top: %dpx\"><a name=\"r%dpara%d\"></a>",
                 (paragraphs[para_index].attributes & 0x7) * 2,
                 record_index, para_index);

        current_link = FALSE;

        /* at the beginning of a paragraph, we start with a clean graphics context */
        current_font = 0;
        current_alignment = 0;
        current_color = 0;
        current_italic = FALSE;
        current_underline = FALSE;
        current_struckthrough = FALSE;
        current_left_margin = 0;
        current_right_margin = 0;

        for (para_start = ptr, textlen = 0; (ptr - para_start) < para_len;) {

            if (*ptr == 0) {
                /* function code */

                if ((ptr - run) > 0) {
                    /* write out any pending text */
                    fwrite (run, 1, (ptr - run), fp);
                    textlen += (ptr - run);
                }

                ptr++;
                fctype = GET_FUNCTION_CODE_TYPE (*ptr);
                fclen = GET_FUNCTION_CODE_DATALEN (*ptr);
                ptr++;

                if (fctype == PLKR_TFC_NEWLINE) {

                    fprintf (fp, "<br />\n");

                }
                else if (fctype == PLKR_TFC_LINK) {

                    int                   record_id, real_record_id,
                        datalen;
                    plkr_DataRecordType   type = 0;
                    unsigned char        *bytes = NULL;
                    char                 *url = NULL;

                    if (fclen == 0) {
                        if (current_link)
                            fprintf (fp, "</a>");
                        current_link = FALSE;
                    }
                    else {
                        record_id = (ptr[0] << 8) + ptr[1];
                        bytes =
                            plkr_GetRecordBytes (doc, record_id, &datalen,
                                                 &type);
                        if (!bytes) {
                            url = plkr_GetRecordURL (doc, record_id);
                        }
                        if (bytes && (type == PLKR_DRTYPE_MAILTO)) {
                            url = MailtoURLFromBytes (bytes, datalen);
                            fprintf (fp, "<a href=\"%s\">", url);
                            free (url);
                            current_link = TRUE;
                        }
                        else if (!bytes && url) {
                            fprintf (fp, "<a href=\"%s\">", url);
                            current_link = TRUE;
                        }
                        else if (bytes && (fclen == 2)) {
                            AddRecord (record_id);
                            real_record_id = GetPageID (record_id);
                            if (type == PLKR_DRTYPE_IMAGE
                                || type == PLKR_DRTYPE_IMAGE_COMPRESSED) {
                                fprintf (fp, "<a href=\"r%d.jpg\">",
                                         real_record_id);
                            }
                            else {
                                fprintf (fp, "<a href=\"r%d.html\">",
                                         real_record_id);
                            }
                            current_link = TRUE;
                        }
                        else if (bytes && (fclen == 4)) {
                            AddRecord (record_id);
                            real_record_id = GetPageID (record_id);
                            fprintf (fp, " <a href=\"r%d.html#r%dpara%d\">",
                                     real_record_id, record_id,
                                     (ptr[2] << 8) + ptr[3]);
                            current_link = TRUE;
                        }
                        else {
                            ShowWarning
                                ("odd link found:  record_id=%d, bytes=0x%p, type=%d, url=%s",
                                 record_id, bytes, type,
                                 (url ? url : "0x0"));
                        }
                    }

                }
                else if (fctype == PLKR_TFC_FONT) {

                    fprintf (fp, "<!-- font change: from %d to %d -->",
                             current_font, *ptr);

                    if (current_font != *ptr) {

                        if (current_font == 1) {
                            fprintf (fp, "</h1>");
                        }
                        else if (current_font == 2) {
                            fprintf (fp, "</h2>");
                        }
                        else if (current_font == 3) {
                            fprintf (fp, "</h3>");
                        }
                        else if (current_font == 4) {
                            fprintf (fp, "</h4>");
                        }
                        else if (current_font == 5) {
                            fprintf (fp, "</h5>");
                        }
                        else if (current_font == 6) {
                            fprintf (fp, "</h6>");
                        }
                        else if (current_font == 7) {
                            fprintf (fp, "</b>");
                        }
                        else if (current_font == 8) {
                            fprintf (fp, "</tt>");
                        }
                        else if (current_font == 11) {
                          fprintf (fp, "</sup>");
                        }

                        if (*ptr == 1) {
                            fprintf (fp, "<h1>");
                        }
                        else if (*ptr == 2) {
                            fprintf (fp, "<h2>");
                        }
                        else if (*ptr == 3) {
                            fprintf (fp, "<h3>");
                        }
                        else if (*ptr == 4) {
                            fprintf (fp, "<h4>");
                        }
                        else if (*ptr == 5) {
                            fprintf (fp, "<h5>");
                        }
                        else if (*ptr == 6) {
                            fprintf (fp, "<h6>");
                        }
                        else if (*ptr == 7) {
                            fprintf (fp, "<b>");
                        }
                        else if (*ptr == 8) {
                            fprintf (fp, "<tt>");
                        }
                        else if (*ptr == 11) {
                          fprintf (fp, "<sup>");
                        }

                        current_font = *ptr;
                    }

                }
                else if (fctype == PLKR_TFC_BITALIC) {

                    fprintf (fp, "<i>");
                    current_italic = TRUE;

                }
                else if (fctype == PLKR_TFC_EITALIC) {

                    if (current_italic) {
                        fprintf (fp, "</i>");
                        current_italic = FALSE;
                    }

                }
                else if (fctype == PLKR_TFC_BULINE) {

                    fprintf (fp, "<u>");
                    current_underline = TRUE;

                }
                else if (fctype == PLKR_TFC_EULINE) {

                    if (current_underline) {
                        fprintf (fp, "</u>");
                        current_underline = FALSE;
                    }

                }
                else if (fctype == PLKR_TFC_BSTRIKE) {

                    fprintf (fp, "<strike>");
                    current_struckthrough = TRUE;

                }
                else if (fctype == PLKR_TFC_ESTRIKE) {

                    if (current_struckthrough) {
                        fprintf (fp, "</strike>");
                        current_struckthrough = FALSE;
                    }

                }
                else if (fctype == PLKR_TFC_HRULE) {

                    fprintf (fp, "<hr size=%d width=", ptr[0]);
                    if (ptr[1] == 0 && ptr[2] > 0)
                        fprintf (fp, "\"%d%%\">\n", ptr[2]);
                    else if (ptr[1] > 0 && ptr[2] == 0)
                        fprintf (fp, "%d>\n", ptr[1]);
                    else
                        fprintf (fp, "\"100%%\">");

                }
                else if (fctype == PLKR_TFC_ALIGN) {

                    if (current_alignment > 0)
                        fprintf (fp, "</div>");
                    current_alignment = 0;
                    if (*ptr < 4) {
                        fprintf (fp, "<div align=\"");
                        if (*ptr == 0)
                            fprintf (fp, "left");
                        else if (*ptr == 1)
                            fprintf (fp, "right");
                        else if (*ptr == 2)
                            fprintf (fp, "center");
                        else if (*ptr == 3)
                            fprintf (fp, "justify");
                        fprintf (fp, "\">");
                        current_alignment = (*ptr) + 1;
                    }

                }
                else if (fctype == PLKR_TFC_MARGINS) {

                    /* Not easy to set, in HTML */
                    fprintf (fp, "<!-- margins:  %d, %d -->", ptr[0],
                             ptr[1]);
#if 0
                    if (current_left_margin != ptr[0]
                        || current_right_margin != ptr[1]) {
                        if (current_right_margin != 0)
                            fprintf (fp, "</td><td width=%d>&nbsp;",
                                     current_right_margin);
                        fprintf (fp, "</td></tr></table>\n");
                    }
                    current_left_margin = ptr[0];
                    current_right_margin = ptr[1];
                    if (current_right_margin > 0
                        || current_left_margin > 0) {
                        fprintf (fp, "<table border=1><tr>");
                        if (current_left_margin != 0) {
                            fprintf (fp, "<td width=%d align=right>",
                                     current_left_margin);
                            if ((ptr - run) > 2) {
                                fwrite (run, 1, ((ptr - 2) - run), fp);
                                textlen += ((ptr - 2) - run);
                            }
                            else {
                                fprintf (fp, "&nbsp;");
                            }
                            fprintf (fp, "</td>");
                        }
                        fprintf (fp, "<td>");
                        if (current_left_margin == 0 && (ptr - run) > 2) {
                            fwrite (run, 1, ((ptr - 2) - run), fp);
                            textlen += ((ptr - 2) - run);
                        }
                    }
                    else {
                        if ((ptr - run) > 2) {
                            fwrite (run, 1, ((ptr - 2) - run), fp);
                            textlen += ((ptr - 2) - run);
                        }
                    }
#endif

                    current_left_margin = ptr[0];
                    current_right_margin = ptr[1];

                }
                else if (fctype == PLKR_TFC_COLOR) {

                    /* not sure what to do here yet */
                    fprintf (fp, "<!-- color=\"#%02x%02x%02x\" -->",
                             ptr[0], ptr[1], ptr[2]);
                    current_color =
                        (ptr[0] << 16) + (ptr[1] << 8) + ptr[2];

                }
                else if (fctype == PLKR_TFC_IMAGE
                         || fctype == PLKR_TFC_IMAGE2) {

                    fprintf (fp, "<IMG SRC=\"r%d.jpg\">",
                             (ptr[0] << 8) + ptr[1]);
                    AddRecord ((ptr[0] << 8) + ptr[1]);

                }
                else if (fctype == PLKR_TFC_TABLE) {

                    int                   record_id, datalen;
                    plkr_DataRecordType   type = 0;
                    unsigned char        *bytes = NULL;

                    record_id = (ptr[0] << 8) + ptr[1];
                    bytes =
                        plkr_GetRecordBytes (doc, record_id, &datalen,
                                             &type);

                    TranscribeTableRecord (doc, record_id, fp, bytes,
                                           datalen, type);

                }
                else if (fctype == PLKR_TFC_UCHAR) {

                    if (fclen == 3)
                        fprintf (fp, "&#%d;", (ptr[1] << 8) + ptr[2]);
                    else if (fclen == 5)
                        fprintf (fp, "&#%d;", (ptr[3] << 8) + ptr[4]);
                    /* skip over alternate text */
                    ptr += ptr[0];

                }
                else {

                    /* ignore function */
                    fprintf (fp, "<!-- function code %d ignored -->",
                             fctype);

                }

                ptr += fclen;
                run = ptr;
            }
            else {
                ptr++;
            }

        }

        if ((ptr - run) > 0) {
            /* output any pending text at the end of the paragraph */
            fwrite (run, 1, (ptr - run), fp);
            textlen += (ptr - run);
            run = ptr;
        }

        if (current_link)
            fprintf (fp, "</a>");

        /* clear the graphics state again */

        if (current_font == 1) {
            fprintf (fp, "</h1>");
        }
        else if (current_font == 2) {
            fprintf (fp, "</h2>");
        }
        else if (current_font == 3) {
            fprintf (fp, "</h3>");
        }
        else if (current_font == 4) {
            fprintf (fp, "</h4>");
        }
        else if (current_font == 5) {
            fprintf (fp, "</h5>");
        }
        else if (current_font == 6) {
            fprintf (fp, "</h6>");
        }
        else if (current_font == 7) {
            fprintf (fp, "</b>");
        }
        else if (current_font == 8) {
            fprintf (fp, "</tt>");
        }
        if (current_italic)
            fprintf (fp, "</i>");
        if (current_underline)
            fprintf (fp, "</u>");
        if (current_struckthrough)
            fprintf (fp, "</strike>");
        if (current_alignment > 0)
            fprintf (fp, "</div>");

#if 0
        if (current_right_margin > 0)
            fprintf (fp, "</td><td width=%d>&nbsp;</td></tr></table>",
                     current_right_margin);
        else if (current_left_margin > 0)
            fprintf (fp, "</td></tr></table>");
#endif

        /* end the paragraph */
        fprintf (fp, "</p>\n");
    }
    free (paragraphs);
    fprintf (fp, "</BODY></HTML>\n");
    return TRUE;
}

static boolean TranscribeRecord
    (
    plkr_Document*  doc,
    char*           dir,
    int             record_index
    )
{
    plkr_DataRecordType  type;
    unsigned char*       data;
    char*                filename;
    int                  data_len;
    FILE*                fp;
    boolean              status = TRUE;

    filename = (char *) malloc (strlen (dir) + 20);

    if ((data =
         plkr_GetRecordBytes (doc, record_index, &data_len,
                              &type)) == NULL) {
        free (filename);
        return FALSE;
    }
    if ((type == PLKR_DRTYPE_TEXT_COMPRESSED) ||
        (type == PLKR_DRTYPE_TEXT)) {
        sprintf (filename, "%s/r%d.html", dir, record_index);
    }
    else if ((type == PLKR_DRTYPE_IMAGE_COMPRESSED) ||
             (type == PLKR_DRTYPE_IMAGE)) {
        sprintf (filename, "%s/r%d.jpg", dir, record_index);
    }
    else if ((type == PLKR_DRTYPE_MULTIIMAGE) ||
             (type == PLKR_DRTYPE_IMAGE)) {
        sprintf (filename, "%s/r%d.jpg", dir, record_index);
    }

    if (type == PLKR_DRTYPE_TEXT_COMPRESSED || type == PLKR_DRTYPE_TEXT) {
        fp = fopen (filename, "w+");
        if (!fp) {
            free (filename);
            return FALSE;
        }
        status =
            TranscribeTextRecord (doc, record_index, fp, data, data_len,
                                  type);
    }
    else if (type == PLKR_DRTYPE_IMAGE_COMPRESSED
             || type == PLKR_DRTYPE_IMAGE) {
        fp = fopen (filename, "wb+");
        if (!fp) {
            free (filename);
            return FALSE;
        }
        status =
            TranscribeImageRecord (doc, record_index, fp, data, data_len,
                                   type);
    }
    else if (type == PLKR_DRTYPE_MULTIIMAGE) {
        fp = fopen (filename, "wb+");
        if (!fp) {
            free (filename);
            return FALSE;
        }
        TranscribeMultiImageRecord (doc, record_index, fp, data, data_len,
                                    type);
        status = FALSE;
    }
    else {

        fprintf (stderr, "Invalid record type %d for record %d\n", type,
                 record_index);
        status = FALSE;
    }
    fclose (fp);

    if (status && (record_index == plkr_GetHomeRecordID (doc))) {
        char                 *symlinkname =
            (char *) malloc (strlen (dir) + 20);
        sprintf (symlinkname, "%s/default.html", dir);
        link (filename, symlinkname);
        free (symlinkname);
    }

    MarkRecordDone (record_index);

    free (filename);
    return status;
}


static void FinishDoc
    (
    plkr_Document*  doc
    )
{
    RecordNode*  ptr;

    while (records != NULL) {
        ptr = records;
        records = ptr->next;
        free (ptr);
    }

    plkr_CloseDoc (doc);
}


int main
    (
    int     ac,
    char**  av,
    char**  envp
    )
{
    plkr_Document*  doc;
    int             i;
    struct stat     buf;
    char*           document_path;
    char*           directory = NULL;
    char*           owner_id = NULL;
    boolean         verbose = 0;
    boolean         status;
    char*           usage_format =
        "Usage:  %s [--directory=DIR] [--verbose] DOCUMENT-FILE\n";

    for (i = 1; i < ac; i++) {
        if (strncmp (av[i], "--directory=", 12) == 0) {
            directory = av[i] + 12;
        }
        else if (strncmp (av[i], "--verbose", 9) == 0) {
            verbose = 1;
        }
        else if (strncmp (av[i], "--", 2) == 0) {
            fprintf (stderr, usage_format, av[0]);
            return 1;
        }
        else {
            break;
        }
    }
    if ((ac - i) != 1) {
        fprintf (stderr, usage_format, av[0]);
        return 1;
    }

    document_path = av[i];

    if (directory == NULL) {
        directory = tmpnam ((char *) malloc (L_tmpnam));
    }

    if (stat (directory, &buf) != 0) {
        /* assume it doesn't exist */
        if (mkdir (directory, 0775) != 0) {
            fprintf (stderr, "Can't create directory %s\n", directory);
            return 1;
        }
    }
    else if (!S_ISDIR (buf.st_mode)) {
        fprintf (stderr, "Specified directory %s is not a directory!\n",
                 directory);
        return 1;
    }

    if (stat (document_path, &buf) != 0) {
        fprintf (stderr, "Can't access document %s\n", document_path);
        return 1;
    }
    else if (!S_ISREG (buf.st_mode)) {
        fprintf (stderr, "Document file %s must be a regular file.\n",
                 document_path);
        return 1;
    }

    plkr_ShowMessages (verbose);

    doc = plkr_OpenDBFile (document_path);
    if (!doc) {
        fprintf (stderr, "Error opening document %s\n", document_path);
        return 1;
    }

    AddRecord (plkr_GetHomeRecordID (doc));

    i = GetNextRecordNumber ();
    while (i > 0) {
        status = TranscribeRecord (doc, directory, i);
        i = GetNextRecordNumber ();
    }
    LinkRecords (directory);

    FinishDoc (doc);

    if (ac < 3)
        printf ("%s\n", directory);

    return (!status);
}
