/* Copyright (c) 2001, Stanford University 
 * All rights reserved 
 * 
 * See the file LICENSE.txt for information on redistributing this software.
 * 
 * Stolen from GLT */

#include "chromium.h"
#include "printspu.h"

/* This is a very clever construction that avoids the use of a huge table
 * of symbols.  These tables handle two-byte enum values (i.e. 0x0000-0xffff).
 * The high-order byte is used as an index into the table of entries; the
 * "index" field indicates where, in the table of strings (below), the names
 * starting with value 0xMM00 reside.  The "count" field then indicates
 * how many (subsequent) entries are defined.
 */

typedef struct {
    short  index;
    short  count;
} GLT_enum_tab_ent;

/* The comments at the end of each line are the MSB represented by the last
 * entry on that line.
 */
static GLT_enum_tab_ent __enum_tab_ents[256] = {
    {1442, 10},{2, 5},{7, 8},{15, 9},{24, 13},{37, 6},{43, 5},{48, 8},{56, 2}, // 0x08
    {58, 2},{60, 3},{63, 243},{306, 246},{552, 245},{-1, -1},{-1, -1}, // 0x0f
    {797, 6},{803, 3},{806, 10},{816, 2},{818, 11},{829, 16},{845, 4}, // 0x16
    {849, 3},{852, 3},{855, 11},{866, 1},{867, 3},{870, 3},{873, 2},{875, 4}, // 0x1e
    {879, 4},{883, 4},{887, 2},{889, 2},{891, 1},{892, 3},{895, 3},{898, 2}, // 0x26
    {900, 4},{904, 4},{908, 2},{910, 46},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x2e
    {-1, -1},{956, 6},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x36
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x3e
    {-1, -1},{962, 8},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x46
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x4e
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x56
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x5e
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x66
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x6e
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x76
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x7e
    {-1, -1},{970, 224},{1194, 248},{-1, -1},{-1, -1},{-1, -1},{-1, -1},	// 0x85
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{1452, 255},{1708, 18},{-1, -1},{-1, -1}, // 0x8d
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x95
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0x9d
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xa5
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xad
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xb5
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xbd
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xc5
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xcd
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xd5
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xdd
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xe5
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xed
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xf5
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}, // 0xfd
    {-1, -1},{-1, -1} // 0xff
};

/* String table.  The comments at the end of the line indicate the index of
 * the string, and the enum value associated with it.  These are sorted by
 * enum value.
 */
static char *__enum_str_tab[] = {
    "GL_ZERO",                                         /* 0: 0x0000 */
    "GL_ONE",                                          /* 1: 0x0001 */
    "GL_ACCUM",                                        /* 2: 0x0100 */
    "GL_LOAD",                                         /* 3: 0x0101 */
    "GL_RETURN",                                       /* 4: 0x0102 */
    "GL_MULT",                                         /* 5: 0x0103 */
    "GL_ADD",                                          /* 6: 0x0104 */
    "GL_NEVER",                                        /* 7: 0x0200 */
    "GL_LESS",                                         /* 8: 0x0201 */
    "GL_EQUAL",                                        /* 9: 0x0202 */
    "GL_LEQUAL",                                       /* 10: 0x0203 */
    "GL_GREATER",                                      /* 11: 0x0204 */
    "GL_NOTEQUAL",                                     /* 12: 0x0205 */
    "GL_GEQUAL",                                       /* 13: 0x0206 */
    "GL_ALWAYS",                                       /* 14: 0x0207 */
    "GL_SRC_COLOR",                                    /* 15: 0x0300 */
    "GL_ONE_MINUS_SRC_COLOR",                          /* 16: 0x0301 */
    "GL_SRC_ALPHA",                                    /* 17: 0x0302 */
    "GL_ONE_MINUS_SRC_ALPHA",                          /* 18: 0x0303 */
    "GL_DST_ALPHA",                                    /* 19: 0x0304 */
    "GL_ONE_MINUS_DST_ALPHA",                          /* 20: 0x0305 */
    "GL_DST_COLOR",                                    /* 21: 0x0306 */
    "GL_ONE_MINUS_DST_COLOR",                          /* 22: 0x0307 */
    "GL_SRC_ALPHA_SATURATE",                           /* 23: 0x0308 */
    "GL_FRONT_LEFT",                                   /* 24: 0x0400 */
    "GL_FRONT_RIGHT",                                  /* 25: 0x0401 */
    "GL_BACK_LEFT",                                    /* 26: 0x0402 */
    "GL_BACK_RIGHT",                                   /* 27: 0x0403 */
    "GL_FRONT",                                        /* 28: 0x0404 */
    "GL_BACK",                                         /* 29: 0x0405 */
    "GL_LEFT",                                         /* 30: 0x0406 */
    "GL_RIGHT",                                        /* 31: 0x0407 */
    "GL_FRONT_AND_BACK",                               /* 32: 0x0408 */
    "GL_AUX0",                                         /* 33: 0x0409 */
    "GL_AUX1",                                         /* 34: 0x040a */
    "GL_AUX2",                                         /* 35: 0x040b */
    "GL_AUX3",                                         /* 36: 0x040c */
    "GL_INVALID_ENUM",                                 /* 37: 0x0500 */
    "GL_INVALID_VALUE",                                /* 38: 0x0501 */
    "GL_INVALID_OPERATION",                            /* 39: 0x0502 */
    "GL_STACK_OVERFLOW",                               /* 40: 0x0503 */
    "GL_STACK_UNDERFLOW",                              /* 41: 0x0504 */
    "GL_OUT_OF_MEMORY",                                /* 42: 0x0505 */
    "GL_2D",                                           /* 43: 0x0600 */
    "GL_3D",                                           /* 44: 0x0601 */
    "GL_3D_COLOR",                                     /* 45: 0x0602 */
    "GL_3D_COLOR_TEXTURE",                             /* 46: 0x0603 */
    "GL_4D_COLOR_TEXTURE",                             /* 47: 0x0604 */
    "GL_PASS_THROUGH_TOKEN",                           /* 48: 0x0700 */
    "GL_POINT_TOKEN",                                  /* 49: 0x0701 */
    "GL_LINE_TOKEN",                                   /* 50: 0x0702 */
    "GL_POLYGON_TOKEN",                                /* 51: 0x0703 */
    "GL_BITMAP_TOKEN",                                 /* 52: 0x0704 */
    "GL_DRAW_PIXEL_TOKEN",                             /* 53: 0x0705 */
    "GL_COPY_PIXEL_TOKEN",                             /* 54: 0x0706 */
    "GL_LINE_RESET_TOKEN",                             /* 55: 0x0707 */
    "GL_EXP",                                          /* 56: 0x0800 */
    "GL_EXP2",                                         /* 57: 0x0801 */
    "GL_CW",                                           /* 58: 0x0900 */
    "GL_CCW",                                          /* 59: 0x0901 */
    "GL_COEFF",                                        /* 60: 0x0a00 */
    "GL_ORDER",                                        /* 61: 0x0a01 */
    "GL_DOMAIN",                                       /* 62: 0x0a02 */
    "GL_CURRENT_COLOR",                                /* 63: 0x0b00 */
    "GL_CURRENT_INDEX",                                /* 64: 0x0b01 */
    "GL_CURRENT_NORMAL",                               /* 65: 0x0b02 */
    "GL_CURRENT_TEXTURE_COORDS",                       /* 66: 0x0b03 */
    "GL_CURRENT_RASTER_COLOR",                         /* 67: 0x0b04 */
    "GL_CURRENT_RASTER_INDEX",                         /* 68: 0x0b05 */
    "GL_CURRENT_RASTER_TEXTURE_COORDS",                /* 69: 0x0b06 */
    "GL_CURRENT_RASTER_POSITION",                      /* 70: 0x0b07 */
    "GL_CURRENT_RASTER_POSITION_VALID",                /* 71: 0x0b08 */
    "GL_CURRENT_RASTER_DISTANCE",                      /* 72: 0x0b09 */
    NULL,                                              /* 73: */
    NULL,                                              /* 74: */
    NULL,                                              /* 75: */
    NULL,                                              /* 76: */
    NULL,                                              /* 77: */
    NULL,                                              /* 78: */
    "GL_POINT_SMOOTH",                                 /* 79: 0x0b10 */
    "GL_POINT_SIZE",                                   /* 80: 0x0b11 */
    "GL_POINT_SIZE_RANGE",                             /* 81: 0x0b12 */
    "GL_POINT_SIZE_GRANULARITY",                       /* 82: 0x0b13 */
    NULL,                                              /* 83: */
    NULL,                                              /* 84: */
    NULL,                                              /* 85: */
    NULL,                                              /* 86: */
    NULL,                                              /* 87: */
    NULL,                                              /* 88: */
    NULL,                                              /* 89: */
    NULL,                                              /* 90: */
    NULL,                                              /* 91: */
    NULL,                                              /* 92: */
    NULL,                                              /* 93: */
    NULL,                                              /* 94: */
    "GL_LINE_SMOOTH",                                  /* 95: 0x0b20 */
    "GL_LINE_WIDTH",                                   /* 96: 0x0b21 */
    "GL_LINE_WIDTH_RANGE",                             /* 97: 0x0b22 */
    "GL_LINE_WIDTH_GRANULARITY",                       /* 98: 0x0b23 */
    "GL_LINE_STIPPLE",                                 /* 99: 0x0b24 */
    "GL_LINE_STIPPLE_PATTERN",                         /* 100: 0x0b25 */
    "GL_LINE_STIPPLE_REPEAT",                          /* 101: 0x0b26 */
    NULL,                                              /* 102: */
    NULL,                                              /* 103: */
    NULL,                                              /* 104: */
    NULL,                                              /* 105: */
    NULL,                                              /* 106: */
    NULL,                                              /* 107: */
    NULL,                                              /* 108: */
    NULL,                                              /* 109: */
    NULL,                                              /* 110: */
    "GL_LIST_MODE",                                    /* 111: 0x0b30 */
    "GL_MAX_LIST_NESTING",                             /* 112: 0x0b31 */
    "GL_LIST_BASE",                                    /* 113: 0x0b32 */
    "GL_LIST_INDEX",                                   /* 114: 0x0b33 */
    NULL,                                              /* 115: */
    NULL,                                              /* 116: */
    NULL,                                              /* 117: */
    NULL,                                              /* 118: */
    NULL,                                              /* 119: */
    NULL,                                              /* 120: */
    NULL,                                              /* 121: */
    NULL,                                              /* 122: */
    NULL,                                              /* 123: */
    NULL,                                              /* 124: */
    NULL,                                              /* 125: */
    NULL,                                              /* 126: */
    "GL_POLYGON_MODE",                                 /* 127: 0x0b40 */
    "GL_POLYGON_SMOOTH",                               /* 128: 0x0b41 */
    "GL_POLYGON_STIPPLE",                              /* 129: 0x0b42 */
    "GL_EDGE_FLAG",                                    /* 130: 0x0b43 */
    "GL_CULL_FACE",                                    /* 131: 0x0b44 */
    "GL_CULL_FACE_MODE",                               /* 132: 0x0b45 */
    "GL_FRONT_FACE",                                   /* 133: 0x0b46 */
    NULL,                                              /* 134: */
    NULL,                                              /* 135: */
    NULL,                                              /* 136: */
    NULL,                                              /* 137: */
    NULL,                                              /* 138: */
    NULL,                                              /* 139: */
    NULL,                                              /* 140: */
    NULL,                                              /* 141: */
    NULL,                                              /* 142: */
    "GL_LIGHTING",                                     /* 143: 0x0b50 */
    "GL_LIGHT_MODEL_LOCAL_VIEWER",                     /* 144: 0x0b51 */
    "GL_LIGHT_MODEL_TWO_SIDE",                         /* 145: 0x0b52 */
    "GL_LIGHT_MODEL_AMBIENT",                          /* 146: 0x0b53 */
    "GL_SHADE_MODEL",                                  /* 147: 0x0b54 */
    "GL_COLOR_MATERIAL_FACE",                          /* 148: 0x0b55 */
    "GL_COLOR_MATERIAL_PARAMETER",                     /* 149: 0x0b56 */
    "GL_COLOR_MATERIAL",                               /* 150: 0x0b57 */
    NULL,                                              /* 151: */
    NULL,                                              /* 152: */
    NULL,                                              /* 153: */
    NULL,                                              /* 154: */
    NULL,                                              /* 155: */
    NULL,                                              /* 156: */
    NULL,                                              /* 157: */
    NULL,                                              /* 158: */
    "GL_FOG",                                          /* 159: 0x0b60 */
    "GL_FOG_INDEX",                                    /* 160: 0x0b61 */
    "GL_FOG_DENSITY",                                  /* 161: 0x0b62 */
    "GL_FOG_START",                                    /* 162: 0x0b63 */
    "GL_FOG_END",                                      /* 163: 0x0b64 */
    "GL_FOG_MODE",                                     /* 164: 0x0b65 */
    "GL_FOG_COLOR",                                    /* 165: 0x0b66 */
    NULL,                                              /* 166: */
    NULL,                                              /* 167: */
    NULL,                                              /* 168: */
    NULL,                                              /* 169: */
    NULL,                                              /* 170: */
    NULL,                                              /* 171: */
    NULL,                                              /* 172: */
    NULL,                                              /* 173: */
    NULL,                                              /* 174: */
    "GL_DEPTH_RANGE",                                  /* 175: 0x0b70 */
    "GL_DEPTH_TEST",                                   /* 176: 0x0b71 */
    "GL_DEPTH_WRITEMASK",                              /* 177: 0x0b72 */
    "GL_DEPTH_CLEAR_VALUE",                            /* 178: 0x0b73 */
    "GL_DEPTH_FUNC",                                   /* 179: 0x0b74 */
    NULL,                                              /* 180: */
    NULL,                                              /* 181: */
    NULL,                                              /* 182: */
    NULL,                                              /* 183: */
    NULL,                                              /* 184: */
    NULL,                                              /* 185: */
    NULL,                                              /* 186: */
    NULL,                                              /* 187: */
    NULL,                                              /* 188: */
    NULL,                                              /* 189: */
    NULL,                                              /* 190: */
    "GL_ACCUM_CLEAR_VALUE",                            /* 191: 0x0b80 */
    NULL,                                              /* 192: */
    NULL,                                              /* 193: */
    NULL,                                              /* 194: */
    NULL,                                              /* 195: */
    NULL,                                              /* 196: */
    NULL,                                              /* 197: */
    NULL,                                              /* 198: */
    NULL,                                              /* 199: */
    NULL,                                              /* 200: */
    NULL,                                              /* 201: */
    NULL,                                              /* 202: */
    NULL,                                              /* 203: */
    NULL,                                              /* 204: */
    NULL,                                              /* 205: */
    NULL,                                              /* 206: */
    "GL_STENCIL_TEST",                                 /* 207: 0x0b90 */
    "GL_STENCIL_CLEAR_VALUE",                          /* 208: 0x0b91 */
    "GL_STENCIL_FUNC",                                 /* 209: 0x0b92 */
    "GL_STENCIL_VALUE_MASK",                           /* 210: 0x0b93 */
    "GL_STENCIL_FAIL",                                 /* 211: 0x0b94 */
    "GL_STENCIL_PASS_DEPTH_FAIL",                      /* 212: 0x0b95 */
    "GL_STENCIL_PASS_DEPTH_PASS",                      /* 213: 0x0b96 */
    "GL_STENCIL_REF",                                  /* 214: 0x0b97 */
    "GL_STENCIL_WRITEMASK",                            /* 215: 0x0b98 */
    NULL,                                              /* 216: */
    NULL,                                              /* 217: */
    NULL,                                              /* 218: */
    NULL,                                              /* 219: */
    NULL,                                              /* 220: */
    NULL,                                              /* 221: */
    NULL,                                              /* 222: */
    "GL_MATRIX_MODE",                                  /* 223: 0x0ba0 */
    "GL_NORMALIZE",                                    /* 224: 0x0ba1 */
    "GL_VIEWPORT",                                     /* 225: 0x0ba2 */
    "GL_MODELVIEW_STACK_DEPTH",                        /* 226: 0x0ba3 */
    "GL_PROJECTION_STACK_DEPTH",                       /* 227: 0x0ba4 */
    "GL_TEXTURE_STACK_DEPTH",                          /* 228: 0x0ba5 */
    "GL_MODELVIEW_MATRIX",                             /* 229: 0x0ba6 */
    "GL_PROJECTION_MATRIX",                            /* 230: 0x0ba7 */
    "GL_TEXTURE_MATRIX",                               /* 231: 0x0ba8 */
    NULL,                                              /* 232: */
    NULL,                                              /* 233: */
    NULL,                                              /* 234: */
    NULL,                                              /* 235: */
    NULL,                                              /* 236: */
    NULL,                                              /* 237: */
    NULL,                                              /* 238: */
    "GL_ATTRIB_STACK_DEPTH",                           /* 239: 0x0bb0 */
    "GL_CLIENT_ATTRIB_STACK_DEPTH",                    /* 240: 0x0bb1 */
    NULL,                                              /* 241: */
    NULL,                                              /* 242: */
    NULL,                                              /* 243: */
    NULL,                                              /* 244: */
    NULL,                                              /* 245: */
    NULL,                                              /* 246: */
    NULL,                                              /* 247: */
    NULL,                                              /* 248: */
    NULL,                                              /* 249: */
    NULL,                                              /* 250: */
    NULL,                                              /* 251: */
    NULL,                                              /* 252: */
    NULL,                                              /* 253: */
    NULL,                                              /* 254: */
    "GL_ALPHA_TEST",                                   /* 255: 0x0bc0 */
    "GL_ALPHA_TEST_FUNC",                              /* 256: 0x0bc1 */
    "GL_ALPHA_TEST_REF",                               /* 257: 0x0bc2 */
    NULL,                                              /* 258: */
    NULL,                                              /* 259: */
    NULL,                                              /* 260: */
    NULL,                                              /* 261: */
    NULL,                                              /* 262: */
    NULL,                                              /* 263: */
    NULL,                                              /* 264: */
    NULL,                                              /* 265: */
    NULL,                                              /* 266: */
    NULL,                                              /* 267: */
    NULL,                                              /* 268: */
    NULL,                                              /* 269: */
    NULL,                                              /* 270: */
    "GL_DITHER",                                       /* 271: 0x0bd0 */
    NULL,                                              /* 272: */
    NULL,                                              /* 273: */
    NULL,                                              /* 274: */
    NULL,                                              /* 275: */
    NULL,                                              /* 276: */
    NULL,                                              /* 277: */
    NULL,                                              /* 278: */
    NULL,                                              /* 279: */
    NULL,                                              /* 280: */
    NULL,                                              /* 281: */
    NULL,                                              /* 282: */
    NULL,                                              /* 283: */
    NULL,                                              /* 284: */
    NULL,                                              /* 285: */
    NULL,                                              /* 286: */
    "GL_BLEND_DST",                                    /* 287: 0x0be0 */
    "GL_BLEND_SRC",                                    /* 288: 0x0be1 */
    "GL_BLEND",                                        /* 289: 0x0be2 */
    NULL,                                              /* 290: */
    NULL,                                              /* 291: */
    NULL,                                              /* 292: */
    NULL,                                              /* 293: */
    NULL,                                              /* 294: */
    NULL,                                              /* 295: */
    NULL,                                              /* 296: */
    NULL,                                              /* 297: */
    NULL,                                              /* 298: */
    NULL,                                              /* 299: */
    NULL,                                              /* 300: */
    NULL,                                              /* 301: */
    NULL,                                              /* 302: */
    "GL_LOGIC_OP_MODE",                                /* 303: 0x0bf0 */
    "GL_LOGIC_OP",                                     /* 304: 0x0bf1 */
    "GL_COLOR_LOGIC_OP",                               /* 305: 0x0bf2 */
    "GL_AUX_BUFFERS",                                  /* 306: 0x0c00 */
    "GL_DRAW_BUFFER",                                  /* 307: 0x0c01 */
    "GL_READ_BUFFER",                                  /* 308: 0x0c02 */
    NULL,                                              /* 309: */
    NULL,                                              /* 310: */
    NULL,                                              /* 311: */
    NULL,                                              /* 312: */
    NULL,                                              /* 313: */
    NULL,                                              /* 314: */
    NULL,                                              /* 315: */
    NULL,                                              /* 316: */
    NULL,                                              /* 317: */
    NULL,                                              /* 318: */
    NULL,                                              /* 319: */
    NULL,                                              /* 320: */
    NULL,                                              /* 321: */
    "GL_SCISSOR_BOX",                                  /* 322: 0x0c10 */
    "GL_SCISSOR_TEST",                                 /* 323: 0x0c11 */
    NULL,                                              /* 324: */
    NULL,                                              /* 325: */
    NULL,                                              /* 326: */
    NULL,                                              /* 327: */
    NULL,                                              /* 328: */
    NULL,                                              /* 329: */
    NULL,                                              /* 330: */
    NULL,                                              /* 331: */
    NULL,                                              /* 332: */
    NULL,                                              /* 333: */
    NULL,                                              /* 334: */
    NULL,                                              /* 335: */
    NULL,                                              /* 336: */
    NULL,                                              /* 337: */
    "GL_INDEX_CLEAR_VALUE",                            /* 338: 0x0c20 */
    "GL_INDEX_WRITEMASK",                              /* 339: 0x0c21 */
    "GL_COLOR_CLEAR_VALUE",                            /* 340: 0x0c22 */
    "GL_COLOR_WRITEMASK",                              /* 341: 0x0c23 */
    NULL,                                              /* 342: */
    NULL,                                              /* 343: */
    NULL,                                              /* 344: */
    NULL,                                              /* 345: */
    NULL,                                              /* 346: */
    NULL,                                              /* 347: */
    NULL,                                              /* 348: */
    NULL,                                              /* 349: */
    NULL,                                              /* 350: */
    NULL,                                              /* 351: */
    NULL,                                              /* 352: */
    NULL,                                              /* 353: */
    "GL_INDEX_MODE",                                   /* 354: 0x0c30 */
    "GL_RGBA_MODE",                                    /* 355: 0x0c31 */
    "GL_DOUBLEBUFFER",                                 /* 356: 0x0c32 */
    "GL_STEREO",                                       /* 357: 0x0c33 */
    NULL,                                              /* 358: */
    NULL,                                              /* 359: */
    NULL,                                              /* 360: */
    NULL,                                              /* 361: */
    NULL,                                              /* 362: */
    NULL,                                              /* 363: */
    NULL,                                              /* 364: */
    NULL,                                              /* 365: */
    NULL,                                              /* 366: */
    NULL,                                              /* 367: */
    NULL,                                              /* 368: */
    NULL,                                              /* 369: */
    "GL_RENDER_MODE",                                  /* 370: 0x0c40 */
    NULL,                                              /* 371: */
    NULL,                                              /* 372: */
    NULL,                                              /* 373: */
    NULL,                                              /* 374: */
    NULL,                                              /* 375: */
    NULL,                                              /* 376: */
    NULL,                                              /* 377: */
    NULL,                                              /* 378: */
    NULL,                                              /* 379: */
    NULL,                                              /* 380: */
    NULL,                                              /* 381: */
    NULL,                                              /* 382: */
    NULL,                                              /* 383: */
    NULL,                                              /* 384: */
    NULL,                                              /* 385: */
    "GL_PERSPECTIVE_CORRECTION_HINT",                  /* 386: 0x0c50 */
    "GL_POINT_SMOOTH_HINT",                            /* 387: 0x0c51 */
    "GL_LINE_SMOOTH_HINT",                             /* 388: 0x0c52 */
    "GL_POLYGON_SMOOTH_HINT",                          /* 389: 0x0c53 */
    "GL_FOG_HINT",                                     /* 390: 0x0c54 */
    NULL,                                              /* 391: */
    NULL,                                              /* 392: */
    NULL,                                              /* 393: */
    NULL,                                              /* 394: */
    NULL,                                              /* 395: */
    NULL,                                              /* 396: */
    NULL,                                              /* 397: */
    NULL,                                              /* 398: */
    NULL,                                              /* 399: */
    NULL,                                              /* 400: */
    NULL,                                              /* 401: */
    "GL_TEXTURE_GEN_S",                                /* 402: 0x0c60 */
    "GL_TEXTURE_GEN_T",                                /* 403: 0x0c61 */
    "GL_TEXTURE_GEN_R",                                /* 404: 0x0c62 */
    "GL_TEXTURE_GEN_Q",                                /* 405: 0x0c63 */
    NULL,                                              /* 406: */
    NULL,                                              /* 407: */
    NULL,                                              /* 408: */
    NULL,                                              /* 409: */
    NULL,                                              /* 410: */
    NULL,                                              /* 411: */
    NULL,                                              /* 412: */
    NULL,                                              /* 413: */
    NULL,                                              /* 414: */
    NULL,                                              /* 415: */
    NULL,                                              /* 416: */
    NULL,                                              /* 417: */
    "GL_PIXEL_MAP_I_TO_I",                             /* 418: 0x0c70 */
    "GL_PIXEL_MAP_S_TO_S",                             /* 419: 0x0c71 */
    "GL_PIXEL_MAP_I_TO_R",                             /* 420: 0x0c72 */
    "GL_PIXEL_MAP_I_TO_G",                             /* 421: 0x0c73 */
    "GL_PIXEL_MAP_I_TO_B",                             /* 422: 0x0c74 */
    "GL_PIXEL_MAP_I_TO_A",                             /* 423: 0x0c75 */
    "GL_PIXEL_MAP_R_TO_R",                             /* 424: 0x0c76 */
    "GL_PIXEL_MAP_G_TO_G",                             /* 425: 0x0c77 */
    "GL_PIXEL_MAP_B_TO_B",                             /* 426: 0x0c78 */
    "GL_PIXEL_MAP_A_TO_A",                             /* 427: 0x0c79 */
    NULL,                                              /* 428: */
    NULL,                                              /* 429: */
    NULL,                                              /* 430: */
    NULL,                                              /* 431: */
    NULL,                                              /* 432: */
    NULL,                                              /* 433: */
    NULL,                                              /* 434: */
    NULL,                                              /* 435: */
    NULL,                                              /* 436: */
    NULL,                                              /* 437: */
    NULL,                                              /* 438: */
    NULL,                                              /* 439: */
    NULL,                                              /* 440: */
    NULL,                                              /* 441: */
    NULL,                                              /* 442: */
    NULL,                                              /* 443: */
    NULL,                                              /* 444: */
    NULL,                                              /* 445: */
    NULL,                                              /* 446: */
    NULL,                                              /* 447: */
    NULL,                                              /* 448: */
    NULL,                                              /* 449: */
    NULL,                                              /* 450: */
    NULL,                                              /* 451: */
    NULL,                                              /* 452: */
    NULL,                                              /* 453: */
    NULL,                                              /* 454: */
    NULL,                                              /* 455: */
    NULL,                                              /* 456: */
    NULL,                                              /* 457: */
    NULL,                                              /* 458: */
    NULL,                                              /* 459: */
    NULL,                                              /* 460: */
    NULL,                                              /* 461: */
    NULL,                                              /* 462: */
    NULL,                                              /* 463: */
    NULL,                                              /* 464: */
    NULL,                                              /* 465: */
    NULL,                                              /* 466: */
    NULL,                                              /* 467: */
    NULL,                                              /* 468: */
    NULL,                                              /* 469: */
    NULL,                                              /* 470: */
    NULL,                                              /* 471: */
    NULL,                                              /* 472: */
    NULL,                                              /* 473: */
    NULL,                                              /* 474: */
    NULL,                                              /* 475: */
    NULL,                                              /* 476: */
    NULL,                                              /* 477: */
    NULL,                                              /* 478: */
    NULL,                                              /* 479: */
    NULL,                                              /* 480: */
    NULL,                                              /* 481: */
    "GL_PIXEL_MAP_I_TO_I_SIZE",                        /* 482: 0x0cb0 */
    "GL_PIXEL_MAP_S_TO_S_SIZE",                        /* 483: 0x0cb1 */
    "GL_PIXEL_MAP_I_TO_R_SIZE",                        /* 484: 0x0cb2 */
    "GL_PIXEL_MAP_I_TO_G_SIZE",                        /* 485: 0x0cb3 */
    "GL_PIXEL_MAP_I_TO_B_SIZE",                        /* 486: 0x0cb4 */
    "GL_PIXEL_MAP_I_TO_A_SIZE",                        /* 487: 0x0cb5 */
    "GL_PIXEL_MAP_R_TO_R_SIZE",                        /* 488: 0x0cb6 */
    "GL_PIXEL_MAP_G_TO_G_SIZE",                        /* 489: 0x0cb7 */
    "GL_PIXEL_MAP_B_TO_B_SIZE",                        /* 490: 0x0cb8 */
    "GL_PIXEL_MAP_A_TO_A_SIZE",                        /* 491: 0x0cb9 */
    NULL,                                              /* 492: */
    NULL,                                              /* 493: */
    NULL,                                              /* 494: */
    NULL,                                              /* 495: */
    NULL,                                              /* 496: */
    NULL,                                              /* 497: */
    NULL,                                              /* 498: */
    NULL,                                              /* 499: */
    NULL,                                              /* 500: */
    NULL,                                              /* 501: */
    NULL,                                              /* 502: */
    NULL,                                              /* 503: */
    NULL,                                              /* 504: */
    NULL,                                              /* 505: */
    NULL,                                              /* 506: */
    NULL,                                              /* 507: */
    NULL,                                              /* 508: */
    NULL,                                              /* 509: */
    NULL,                                              /* 510: */
    NULL,                                              /* 511: */
    NULL,                                              /* 512: */
    NULL,                                              /* 513: */
    NULL,                                              /* 514: */
    NULL,                                              /* 515: */
    NULL,                                              /* 516: */
    NULL,                                              /* 517: */
    NULL,                                              /* 518: */
    NULL,                                              /* 519: */
    NULL,                                              /* 520: */
    NULL,                                              /* 521: */
    NULL,                                              /* 522: */
    NULL,                                              /* 523: */
    NULL,                                              /* 524: */
    NULL,                                              /* 525: */
    NULL,                                              /* 526: */
    NULL,                                              /* 527: */
    NULL,                                              /* 528: */
    NULL,                                              /* 529: */
    NULL,                                              /* 530: */
    NULL,                                              /* 531: */
    NULL,                                              /* 532: */
    NULL,                                              /* 533: */
    NULL,                                              /* 534: */
    NULL,                                              /* 535: */
    NULL,                                              /* 536: */
    NULL,                                              /* 537: */
    NULL,                                              /* 538: */
    NULL,                                              /* 539: */
    NULL,                                              /* 540: */
    NULL,                                              /* 541: */
    NULL,                                              /* 542: */
    NULL,                                              /* 543: */
    NULL,                                              /* 544: */
    NULL,                                              /* 545: */
    "GL_UNPACK_SWAP_BYTES",                            /* 546: 0x0cf0 */
    "GL_UNPACK_LSB_FIRST",                             /* 547: 0x0cf1 */
    "GL_UNPACK_ROW_LENGTH",                            /* 548: 0x0cf2 */
    "GL_UNPACK_SKIP_ROWS",                             /* 549: 0x0cf3 */
    "GL_UNPACK_SKIP_PIXELS",                           /* 550: 0x0cf4 */
    "GL_UNPACK_ALIGNMENT",                             /* 551: 0x0cf5 */
    "GL_PACK_SWAP_BYTES",                              /* 552: 0x0d00 */
    "GL_PACK_LSB_FIRST",                               /* 553: 0x0d01 */
    "GL_PACK_ROW_LENGTH",                              /* 554: 0x0d02 */
    "GL_PACK_SKIP_ROWS",                               /* 555: 0x0d03 */
    "GL_PACK_SKIP_PIXELS",                             /* 556: 0x0d04 */
    "GL_PACK_ALIGNMENT",                               /* 557: 0x0d05 */
    NULL,                                              /* 558: */
    NULL,                                              /* 559: */
    NULL,                                              /* 560: */
    NULL,                                              /* 561: */
    NULL,                                              /* 562: */
    NULL,                                              /* 563: */
    NULL,                                              /* 564: */
    NULL,                                              /* 565: */
    NULL,                                              /* 566: */
    NULL,                                              /* 567: */
    "GL_MAP_COLOR",                                    /* 568: 0x0d10 */
    "GL_MAP_STENCIL",                                  /* 569: 0x0d11 */
    "GL_INDEX_SHIFT",                                  /* 570: 0x0d12 */
    "GL_INDEX_OFFSET",                                 /* 571: 0x0d13 */
    "GL_RED_SCALE",                                    /* 572: 0x0d14 */
    "GL_RED_BIAS",                                     /* 573: 0x0d15 */
    "GL_ZOOM_X",                                       /* 574: 0x0d16 */
    "GL_ZOOM_Y",                                       /* 575: 0x0d17 */
    "GL_GREEN_SCALE",                                  /* 576: 0x0d18 */
    "GL_GREEN_BIAS",                                   /* 577: 0x0d19 */
    "GL_BLUE_SCALE",                                   /* 578: 0x0d1a */
    "GL_BLUE_BIAS",                                    /* 579: 0x0d1b */
    "GL_ALPHA_SCALE",                                  /* 580: 0x0d1c */
    "GL_ALPHA_BIAS",                                   /* 581: 0x0d1d */
    "GL_DEPTH_SCALE",                                  /* 582: 0x0d1e */
    "GL_DEPTH_BIAS",                                   /* 583: 0x0d1f */
    NULL,                                              /* 584: */
    NULL,                                              /* 585: */
    NULL,                                              /* 586: */
    NULL,                                              /* 587: */
    NULL,                                              /* 588: */
    NULL,                                              /* 589: */
    NULL,                                              /* 590: */
    NULL,                                              /* 591: */
    NULL,                                              /* 592: */
    NULL,                                              /* 593: */
    NULL,                                              /* 594: */
    NULL,                                              /* 595: */
    NULL,                                              /* 596: */
    NULL,                                              /* 597: */
    NULL,                                              /* 598: */
    NULL,                                              /* 599: */
    "GL_MAX_EVAL_ORDER",                               /* 600: 0x0d30 */
    "GL_MAX_LIGHTS",                                   /* 601: 0x0d31 */
    "GL_MAX_CLIP_PLANES",                              /* 602: 0x0d32 */
    "GL_MAX_TEXTURE_SIZE",                             /* 603: 0x0d33 */
    "GL_MAX_PIXEL_MAP_TABLE",                          /* 604: 0x0d34 */
    "GL_MAX_ATTRIB_STACK_DEPTH",                       /* 605: 0x0d35 */
    "GL_MAX_MODELVIEW_STACK_DEPTH",                    /* 606: 0x0d36 */
    "GL_MAX_NAME_STACK_DEPTH",                         /* 607: 0x0d37 */
    "GL_MAX_PROJECTION_STACK_DEPTH",                   /* 608: 0x0d38 */
    "GL_MAX_TEXTURE_STACK_DEPTH",                      /* 609: 0x0d39 */
    "GL_MAX_VIEWPORT_DIMS",                            /* 610: 0x0d3a */
    "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH",                /* 611: 0x0d3b */
    NULL,                                              /* 612: */
    NULL,                                              /* 613: */
    NULL,                                              /* 614: */
    NULL,                                              /* 615: */
    NULL,                                              /* 616: */
    NULL,                                              /* 617: */
    NULL,                                              /* 618: */
    NULL,                                              /* 619: */
    NULL,                                              /* 620: */
    NULL,                                              /* 621: */
    NULL,                                              /* 622: */
    NULL,                                              /* 623: */
    NULL,                                              /* 624: */
    NULL,                                              /* 625: */
    NULL,                                              /* 626: */
    NULL,                                              /* 627: */
    NULL,                                              /* 628: */
    NULL,                                              /* 629: */
    NULL,                                              /* 630: */
    NULL,                                              /* 631: */
    "GL_SUBPIXEL_BITS",                                /* 632: 0x0d50 */
    "GL_INDEX_BITS",                                   /* 633: 0x0d51 */
    "GL_RED_BITS",                                     /* 634: 0x0d52 */
    "GL_GREEN_BITS",                                   /* 635: 0x0d53 */
    "GL_BLUE_BITS",                                    /* 636: 0x0d54 */
    "GL_ALPHA_BITS",                                   /* 637: 0x0d55 */
    "GL_DEPTH_BITS",                                   /* 638: 0x0d56 */
    "GL_STENCIL_BITS",                                 /* 639: 0x0d57 */
    "GL_ACCUM_RED_BITS",                               /* 640: 0x0d58 */
    "GL_ACCUM_GREEN_BITS",                             /* 641: 0x0d59 */
    "GL_ACCUM_BLUE_BITS",                              /* 642: 0x0d5a */
    "GL_ACCUM_ALPHA_BITS",                             /* 643: 0x0d5b */
    NULL,                                              /* 644: */
    NULL,                                              /* 645: */
    NULL,                                              /* 646: */
    NULL,                                              /* 647: */
    NULL,                                              /* 648: */
    NULL,                                              /* 649: */
    NULL,                                              /* 650: */
    NULL,                                              /* 651: */
    NULL,                                              /* 652: */
    NULL,                                              /* 653: */
    NULL,                                              /* 654: */
    NULL,                                              /* 655: */
    NULL,                                              /* 656: */
    NULL,                                              /* 657: */
    NULL,                                              /* 658: */
    NULL,                                              /* 659: */
    NULL,                                              /* 660: */
    NULL,                                              /* 661: */
    NULL,                                              /* 662: */
    NULL,                                              /* 663: */
    "GL_NAME_STACK_DEPTH",                             /* 664: 0x0d70 */
    NULL,                                              /* 665: */
    NULL,                                              /* 666: */
    NULL,                                              /* 667: */
    NULL,                                              /* 668: */
    NULL,                                              /* 669: */
    NULL,                                              /* 670: */
    NULL,                                              /* 671: */
    NULL,                                              /* 672: */
    NULL,                                              /* 673: */
    NULL,                                              /* 674: */
    NULL,                                              /* 675: */
    NULL,                                              /* 676: */
    NULL,                                              /* 677: */
    NULL,                                              /* 678: */
    NULL,                                              /* 679: */
    "GL_AUTO_NORMAL",                                  /* 680: 0x0d80 */
    NULL,                                              /* 681: */
    NULL,                                              /* 682: */
    NULL,                                              /* 683: */
    NULL,                                              /* 684: */
    NULL,                                              /* 685: */
    NULL,                                              /* 686: */
    NULL,                                              /* 687: */
    NULL,                                              /* 688: */
    NULL,                                              /* 689: */
    NULL,                                              /* 690: */
    NULL,                                              /* 691: */
    NULL,                                              /* 692: */
    NULL,                                              /* 693: */
    NULL,                                              /* 694: */
    NULL,                                              /* 695: */
    "GL_MAP1_COLOR_4",                                 /* 696: 0x0d90 */
    "GL_MAP1_INDEX",                                   /* 697: 0x0d91 */
    "GL_MAP1_NORMAL",                                  /* 698: 0x0d92 */
    "GL_MAP1_TEXTURE_COORD_1",                         /* 699: 0x0d93 */
    "GL_MAP1_TEXTURE_COORD_2",                         /* 700: 0x0d94 */
    "GL_MAP1_TEXTURE_COORD_3",                         /* 701: 0x0d95 */
    "GL_MAP1_TEXTURE_COORD_4",                         /* 702: 0x0d96 */
    "GL_MAP1_VERTEX_3",                                /* 703: 0x0d97 */
    "GL_MAP1_VERTEX_4",                                /* 704: 0x0d98 */
    NULL,                                              /* 705: */
    NULL,                                              /* 706: */
    NULL,                                              /* 707: */
    NULL,                                              /* 708: */
    NULL,                                              /* 709: */
    NULL,                                              /* 710: */
    NULL,                                              /* 711: */
    NULL,                                              /* 712: */
    NULL,                                              /* 713: */
    NULL,                                              /* 714: */
    NULL,                                              /* 715: */
    NULL,                                              /* 716: */
    NULL,                                              /* 717: */
    NULL,                                              /* 718: */
    NULL,                                              /* 719: */
    NULL,                                              /* 720: */
    NULL,                                              /* 721: */
    NULL,                                              /* 722: */
    NULL,                                              /* 723: */
    NULL,                                              /* 724: */
    NULL,                                              /* 725: */
    NULL,                                              /* 726: */
    NULL,                                              /* 727: */
    "GL_MAP2_COLOR_4",                                 /* 728: 0x0db0 */
    "GL_MAP2_INDEX",                                   /* 729: 0x0db1 */
    "GL_MAP2_NORMAL",                                  /* 730: 0x0db2 */
    "GL_MAP2_TEXTURE_COORD_1",                         /* 731: 0x0db3 */
    "GL_MAP2_TEXTURE_COORD_2",                         /* 732: 0x0db4 */
    "GL_MAP2_TEXTURE_COORD_3",                         /* 733: 0x0db5 */
    "GL_MAP2_TEXTURE_COORD_4",                         /* 734: 0x0db6 */
    "GL_MAP2_VERTEX_3",                                /* 735: 0x0db7 */
    "GL_MAP2_VERTEX_4",                                /* 736: 0x0db8 */
    NULL,                                              /* 737: */
    NULL,                                              /* 738: */
    NULL,                                              /* 739: */
    NULL,                                              /* 740: */
    NULL,                                              /* 741: */
    NULL,                                              /* 742: */
    NULL,                                              /* 743: */
    NULL,                                              /* 744: */
    NULL,                                              /* 745: */
    NULL,                                              /* 746: */
    NULL,                                              /* 747: */
    NULL,                                              /* 748: */
    NULL,                                              /* 749: */
    NULL,                                              /* 750: */
    NULL,                                              /* 751: */
    NULL,                                              /* 752: */
    NULL,                                              /* 753: */
    NULL,                                              /* 754: */
    NULL,                                              /* 755: */
    NULL,                                              /* 756: */
    NULL,                                              /* 757: */
    NULL,                                              /* 758: */
    NULL,                                              /* 759: */
    "GL_MAP1_GRID_DOMAIN",                             /* 760: 0x0dd0 */
    "GL_MAP1_GRID_SEGMENTS",                           /* 761: 0x0dd1 */
    "GL_MAP2_GRID_DOMAIN",                             /* 762: 0x0dd2 */
    "GL_MAP2_GRID_SEGMENTS",                           /* 763: 0x0dd3 */
    NULL,                                              /* 764: */
    NULL,                                              /* 765: */
    NULL,                                              /* 766: */
    NULL,                                              /* 767: */
    NULL,                                              /* 768: */
    NULL,                                              /* 769: */
    NULL,                                              /* 770: */
    NULL,                                              /* 771: */
    NULL,                                              /* 772: */
    NULL,                                              /* 773: */
    NULL,                                              /* 774: */
    NULL,                                              /* 775: */
    "GL_TEXTURE_1D",                                   /* 776: 0x0de0 */
    "GL_TEXTURE_2D",                                   /* 777: 0x0de1 */
    NULL,                                              /* 778: */
    NULL,                                              /* 779: */
    NULL,                                              /* 780: */
    NULL,                                              /* 781: */
    NULL,                                              /* 782: */
    NULL,                                              /* 783: */
    NULL,                                              /* 784: */
    NULL,                                              /* 785: */
    NULL,                                              /* 786: */
    NULL,                                              /* 787: */
    NULL,                                              /* 788: */
    NULL,                                              /* 789: */
    NULL,                                              /* 790: */
    NULL,                                              /* 791: */
    "GL_FEEDBACK_BUFFER_POINTER",                      /* 792: 0x0df0 */
    "GL_FEEDBACK_BUFFER_SIZE",                         /* 793: 0x0df1 */
    "GL_FEEDBACK_BUFFER_TYPE",                         /* 794: 0x0df2 */
    "GL_SELECTION_BUFFER_POINTER",                     /* 795: 0x0df3 */
    "GL_SELECTION_BUFFER_SIZE",                        /* 796: 0x0df4 */
    "GL_TEXTURE_WIDTH",                                /* 797: 0x1000 */
    "GL_TEXTURE_HEIGHT",                               /* 798: 0x1001 */
    NULL,                                              /* 799: */
    "GL_TEXTURE_COMPONENTS",                           /* 800: 0x1003 */
    "GL_TEXTURE_BORDER_COLOR",                         /* 801: 0x1004 */
    "GL_TEXTURE_BORDER",                               /* 802: 0x1005 */
    "GL_DONT_CARE",                                    /* 803: 0x1100 */
    "GL_FASTEST",                                      /* 804: 0x1101 */
    "GL_NICEST",                                       /* 805: 0x1102 */
    "GL_AMBIENT",                                      /* 806: 0x1200 */
    "GL_DIFFUSE",                                      /* 807: 0x1201 */
    "GL_SPECULAR",                                     /* 808: 0x1202 */
    "GL_POSITION",                                     /* 809: 0x1203 */
    "GL_SPOT_DIRECTION",                               /* 810: 0x1204 */
    "GL_SPOT_EXPONENT",                                /* 811: 0x1205 */
    "GL_SPOT_CUTOFF",                                  /* 812: 0x1206 */
    "GL_CONSTANT_ATTENUATION",                         /* 813: 0x1207 */
    "GL_LINEAR_ATTENUATION",                           /* 814: 0x1208 */
    "GL_QUADRATIC_ATTENUATION",                        /* 815: 0x1209 */
    "GL_COMPILE",                                      /* 816: 0x1300 */
    "GL_COMPILE_AND_EXECUTE",                          /* 817: 0x1301 */
    "GL_BYTE",                                         /* 818: 0x1400 */
    "GL_UNSIGNED_BYTE",                                /* 819: 0x1401 */
    "GL_SHORT",                                        /* 820: 0x1402 */
    "GL_UNSIGNED_SHORT",                               /* 821: 0x1403 */
    "GL_INT",                                          /* 822: 0x1404 */
    "GL_UNSIGNED_INT",                                 /* 823: 0x1405 */
    "GL_FLOAT",                                        /* 824: 0x1406 */
    "GL_2_BYTES",                                      /* 825: 0x1407 */
    "GL_3_BYTES",                                      /* 826: 0x1408 */
    "GL_4_BYTES",                                      /* 827: 0x1409 */
    "GL_DOUBLE_EXT",                                   /* 828: 0x140a */
    "GL_CLEAR",                                        /* 829: 0x1500 */
    "GL_AND",                                          /* 830: 0x1501 */
    "GL_AND_REVERSE",                                  /* 831: 0x1502 */
    "GL_COPY",                                         /* 832: 0x1503 */
    "GL_AND_INVERTED",                                 /* 833: 0x1504 */
    "GL_NOOP",                                         /* 834: 0x1505 */
    "GL_XOR",                                          /* 835: 0x1506 */
    "GL_OR",                                           /* 836: 0x1507 */
    "GL_NOR",                                          /* 837: 0x1508 */
    "GL_EQUIV",                                        /* 838: 0x1509 */
    "GL_INVERT",                                       /* 839: 0x150a */
    "GL_OR_REVERSE",                                   /* 840: 0x150b */
    "GL_COPY_INVERTED",                                /* 841: 0x150c */
    "GL_OR_INVERTED",                                  /* 842: 0x150d */
    "GL_NAND",                                         /* 843: 0x150e */
    "GL_SET",                                          /* 844: 0x150f */
    "GL_EMISSION",                                     /* 845: 0x1600 */
    "GL_SHININESS",                                    /* 846: 0x1601 */
    "GL_AMBIENT_AND_DIFFUSE",                          /* 847: 0x1602 */
    "GL_COLOR_INDEXES",                                /* 848: 0x1603 */
    "GL_MODELVIEW",                                    /* 849: 0x1700 */
    "GL_PROJECTION",                                   /* 850: 0x1701 */
    "GL_TEXTURE",                                      /* 851: 0x1702 */
    "GL_COLOR",                                        /* 852: 0x1800 */
    "GL_DEPTH",                                        /* 853: 0x1801 */
    "GL_STENCIL",                                      /* 854: 0x1802 */
    "GL_COLOR_INDEX",                                  /* 855: 0x1900 */
    "GL_STENCIL_INDEX",                                /* 856: 0x1901 */
    "GL_DEPTH_COMPONENT",                              /* 857: 0x1902 */
    "GL_RED",                                          /* 858: 0x1903 */
    "GL_GREEN",                                        /* 859: 0x1904 */
    "GL_BLUE",                                         /* 860: 0x1905 */
    "GL_ALPHA",                                        /* 861: 0x1906 */
    "GL_RGB",                                          /* 862: 0x1907 */
    "GL_RGBA",                                         /* 863: 0x1908 */
    "GL_LUMINANCE",                                    /* 864: 0x1909 */
    "GL_LUMINANCE_ALPHA",                              /* 865: 0x190a */
    "GL_BITMAP",                                       /* 866: 0x1a00 */
    "GL_POINT",                                        /* 867: 0x1b00 */
    "GL_LINE",                                         /* 868: 0x1b01 */
    "GL_FILL",                                         /* 869: 0x1b02 */
    "GL_RENDER",                                       /* 870: 0x1c00 */
    "GL_FEEDBACK",                                     /* 871: 0x1c01 */
    "GL_SELECT",                                       /* 872: 0x1c02 */
    "GL_FLAT",                                         /* 873: 0x1d00 */
    "GL_SMOOTH",                                       /* 874: 0x1d01 */
    "GL_KEEP",                                         /* 875: 0x1e00 */
    "GL_REPLACE",                                      /* 876: 0x1e01 */
    "GL_INCR",                                         /* 877: 0x1e02 */
    "GL_DECR",                                         /* 878: 0x1e03 */
    "GL_VENDOR",                                       /* 879: 0x1f00 */
    "GL_RENDERER",                                     /* 880: 0x1f01 */
    "GL_VERSION",                                      /* 881: 0x1f02 */
    "GL_EXTENSIONS",                                   /* 882: 0x1f03 */
    "GL_S",                                            /* 883: 0x2000 */
    "GL_T",                                            /* 884: 0x2001 */
    "GL_R",                                            /* 885: 0x2002 */
    "GL_Q",                                            /* 886: 0x2003 */
    "GL_MODULATE",                                     /* 887: 0x2100 */
    "GL_DECAL",                                        /* 888: 0x2101 */
    "GL_TEXTURE_ENV_MODE",                             /* 889: 0x2200 */
    "GL_TEXTURE_ENV_COLOR",                            /* 890: 0x2201 */
    "GL_TEXTURE_ENV",                                  /* 891: 0x2300 */
    "GL_EYE_LINEAR",                                   /* 892: 0x2400 */
    "GL_OBJECT_LINEAR",                                /* 893: 0x2401 */
    "GL_SPHERE_MAP",                                   /* 894: 0x2402 */
    "GL_TEXTURE_GEN_MODE",                             /* 895: 0x2500 */
    "GL_OBJECT_PLANE",                                 /* 896: 0x2501 */
    "GL_EYE_PLANE",                                    /* 897: 0x2502 */
    "GL_NEAREST",                                      /* 898: 0x2600 */
    "GL_LINEAR",                                       /* 899: 0x2601 */
    "GL_NEAREST_MIPMAP_NEAREST",                       /* 900: 0x2700 */
    "GL_LINEAR_MIPMAP_NEAREST",                        /* 901: 0x2701 */
    "GL_NEAREST_MIPMAP_LINEAR",                        /* 902: 0x2702 */
    "GL_LINEAR_MIPMAP_LINEAR",                         /* 903: 0x2703 */
    "GL_TEXTURE_MAG_FILTER",                           /* 904: 0x2800 */
    "GL_TEXTURE_MIN_FILTER",                           /* 905: 0x2801 */
    "GL_TEXTURE_WRAP_S",                               /* 906: 0x2802 */
    "GL_TEXTURE_WRAP_T",                               /* 907: 0x2803 */
    "GL_CLAMP",                                        /* 908: 0x2900 */
    "GL_REPEAT",                                       /* 909: 0x2901 */
    "GL_POLYGON_OFFSET_UNITS",                         /* 910: 0x2a00 */
    "GL_POLYGON_OFFSET_POINT",                         /* 911: 0x2a01 */
    "GL_POLYGON_OFFSET_LINE",                          /* 912: 0x2a02 */
    NULL,                                              /* 913: */
    NULL,                                              /* 914: */
    NULL,                                              /* 915: */
    NULL,                                              /* 916: */
    NULL,                                              /* 917: */
    NULL,                                              /* 918: */
    NULL,                                              /* 919: */
    NULL,                                              /* 920: */
    NULL,                                              /* 921: */
    NULL,                                              /* 922: */
    NULL,                                              /* 923: */
    NULL,                                              /* 924: */
    NULL,                                              /* 925: */
    "GL_R3_G3_B2",                                     /* 926: 0x2a10 */
    NULL,                                              /* 927: */
    NULL,                                              /* 928: */
    NULL,                                              /* 929: */
    NULL,                                              /* 930: */
    NULL,                                              /* 931: */
    NULL,                                              /* 932: */
    NULL,                                              /* 933: */
    NULL,                                              /* 934: */
    NULL,                                              /* 935: */
    NULL,                                              /* 936: */
    NULL,                                              /* 937: */
    NULL,                                              /* 938: */
    NULL,                                              /* 939: */
    NULL,                                              /* 940: */
    NULL,                                              /* 941: */
    "GL_V2F",                                          /* 942: 0x2a20 */
    "GL_V3F",                                          /* 943: 0x2a21 */
    "GL_C4UB_V2F",                                     /* 944: 0x2a22 */
    "GL_C4UB_V3F",                                     /* 945: 0x2a23 */
    "GL_C3F_V3F",                                      /* 946: 0x2a24 */
    "GL_N3F_V3F",                                      /* 947: 0x2a25 */
    "GL_C4F_N3F_V3F",                                  /* 948: 0x2a26 */
    "GL_T2F_V3F",                                      /* 949: 0x2a27 */
    "GL_T4F_V4F",                                      /* 950: 0x2a28 */
    "GL_T2F_C4UB_V3F",                                 /* 951: 0x2a29 */
    "GL_T2F_C3F_V3F",                                  /* 952: 0x2a2a */
    "GL_T2F_N3F_V3F",                                  /* 953: 0x2a2b */
    "GL_T2F_C4F_N3F_V3F",                              /* 954: 0x2a2c */
    "GL_T4F_C4F_N3F_V4F",                              /* 955: 0x2a2d */
    "GL_CLIP_PLANE0",                                  /* 956: 0x3000 */
    "GL_CLIP_PLANE1",                                  /* 957: 0x3001 */
    "GL_CLIP_PLANE2",                                  /* 958: 0x3002 */
    "GL_CLIP_PLANE3",                                  /* 959: 0x3003 */
    "GL_CLIP_PLANE4",                                  /* 960: 0x3004 */
    "GL_CLIP_PLANE5",                                  /* 961: 0x3005 */
    "GL_LIGHT0",                                       /* 962: 0x4000 */
    "GL_LIGHT1",                                       /* 963: 0x4001 */
    "GL_LIGHT2",                                       /* 964: 0x4002 */
    "GL_LIGHT3",                                       /* 965: 0x4003 */
    "GL_LIGHT4",                                       /* 966: 0x4004 */
    "GL_LIGHT5",                                       /* 967: 0x4005 */
    "GL_LIGHT6",                                       /* 968: 0x4006 */
    "GL_LIGHT7",                                       /* 969: 0x4007 */
    "GL_ABGR_EXT",                                     /* 970: 0x8000 */
    "GL_CONSTANT_COLOR_EXT",                           /* 971: 0x8001 */
    "GL_ONE_MINUS_CONSTANT_COLOR_EXT",                 /* 972: 0x8002 */
    "GL_CONSTANT_ALPHA_EXT",                           /* 973: 0x8003 */
    "GL_ONE_MINUS_CONSTANT_ALPHA_EXT",                 /* 974: 0x8004 */
    "GL_BLEND_COLOR_EXT",                              /* 975: 0x8005 */
    "GL_FUNC_ADD_EXT",                                 /* 976: 0x8006 */
    "GL_MIN_EXT",                                      /* 977: 0x8007 */
    "GL_MAX_EXT",                                      /* 978: 0x8008 */
    "GL_BLEND_EQUATION_EXT",                           /* 979: 0x8009 */
    "GL_FUNC_SUBTRACT_EXT",                            /* 980: 0x800a */
    "GL_FUNC_REVERSE_SUBTRACT_EXT",                    /* 981: 0x800b */
    "GL_CMYK_EXT",                                     /* 982: 0x800c */
    "GL_CMYKA_EXT",                                    /* 983: 0x800d */
    "GL_PACK_CMYK_HINT_EXT",                           /* 984: 0x800e */
    "GL_UNPACK_CMYK_HINT_EXT",                         /* 985: 0x800f */
    "GL_CONVOLUTION_1D_EXT",                           /* 986: 0x8010 */
    "GL_CONVOLUTION_2D_EXT",                           /* 987: 0x8011 */
    "GL_SEPARABLE_2D_EXT",                             /* 988: 0x8012 */
    "GL_CONVOLUTION_BORDER_MODE_EXT",                  /* 989: 0x8013 */
    "GL_CONVOLUTION_FILTER_SCALE_EXT",                 /* 990: 0x8014 */
    "GL_CONVOLUTION_FILTER_BIAS_EXT",                  /* 991: 0x8015 */
    "GL_REDUCE_EXT",                                   /* 992: 0x8016 */
    "GL_CONVOLUTION_FORMAT_EXT",                       /* 993: 0x8017 */
    "GL_CONVOLUTION_WIDTH_EXT",                        /* 994: 0x8018 */
    "GL_CONVOLUTION_HEIGHT_EXT",                       /* 995: 0x8019 */
    "GL_MAX_CONVOLUTION_WIDTH_EXT",                    /* 996: 0x801a */
    "GL_MAX_CONVOLUTION_HEIGHT_EXT",                   /* 997: 0x801b */
    "GL_POST_CONVOLUTION_RED_SCALE_EXT",               /* 998: 0x801c */
    "GL_POST_CONVOLUTION_GREEN_SCALE_EXT",             /* 999: 0x801d */
    "GL_POST_CONVOLUTION_BLUE_SCALE_EXT",              /* 1000: 0x801e */
    "GL_POST_CONVOLUTION_ALPHA_SCALE_EXT",             /* 1001: 0x801f */
    "GL_POST_CONVOLUTION_RED_BIAS_EXT",                /* 1002: 0x8020 */
    "GL_POST_CONVOLUTION_GREEN_BIAS_EXT",              /* 1003: 0x8021 */
    "GL_POST_CONVOLUTION_BLUE_BIAS_EXT",               /* 1004: 0x8022 */
    "GL_POST_CONVOLUTION_ALPHA_BIAS_EXT",              /* 1005: 0x8023 */
    "GL_HISTOGRAM_EXT",                                /* 1006: 0x8024 */
    "GL_PROXY_HISTOGRAM_EXT",                          /* 1007: 0x8025 */
    "GL_HISTOGRAM_WIDTH_EXT",                          /* 1008: 0x8026 */
    "GL_HISTOGRAM_FORMAT_EXT",                         /* 1009: 0x8027 */
    "GL_HISTOGRAM_RED_SIZE_EXT",                       /* 1010: 0x8028 */
    "GL_HISTOGRAM_GREEN_SIZE_EXT",                     /* 1011: 0x8029 */
    "GL_HISTOGRAM_BLUE_SIZE_EXT",                      /* 1012: 0x802a */
    "GL_HISTOGRAM_ALPHA_SIZE_EXT",                     /* 1013: 0x802b */
    "GL_HISTOGRAM_LUMINANCE_SIZE_EXT",                 /* 1014: 0x802c */
    "GL_HISTOGRAM_SINK_EXT",                           /* 1015: 0x802d */
    "GL_MINMAX_EXT",                                   /* 1016: 0x802e */
    "GL_MINMAX_FORMAT_EXT",                            /* 1017: 0x802f */
    "GL_MINMAX_SINK_EXT",                              /* 1018: 0x8030 */
    "GL_TABLE_TOO_LARGE_EXT",                          /* 1019: 0x8031 */
    "GL_UNSIGNED_BYTE_3_3_2_EXT",                      /* 1020: 0x8032 */
    "GL_UNSIGNED_SHORT_4_4_4_4_EXT",                   /* 1021: 0x8033 */
    "GL_UNSIGNED_SHORT_5_5_5_1_EXT",                   /* 1022: 0x8034 */
    "GL_UNSIGNED_INT_8_8_8_8_EXT",                     /* 1023: 0x8035 */
    "GL_UNSIGNED_INT_10_10_10_2_EXT",                  /* 1024: 0x8036 */
    "GL_POLYGON_OFFSET_EXT",                           /* 1025: 0x8037 */
    "GL_POLYGON_OFFSET_FACTOR_EXT",                    /* 1026: 0x8038 */
    "GL_POLYGON_OFFSET_BIAS_EXT",                      /* 1027: 0x8039 */
    "GL_RESCALE_NORMAL_EXT",                           /* 1028: 0x803a */
    "GL_ALPHA4_EXT",                                   /* 1029: 0x803b */
    "GL_ALPHA8_EXT",                                   /* 1030: 0x803c */
    "GL_ALPHA12_EXT",                                  /* 1031: 0x803d */
    "GL_ALPHA16_EXT",                                  /* 1032: 0x803e */
    "GL_LUMINANCE4_EXT",                               /* 1033: 0x803f */
    "GL_LUMINANCE8_EXT",                               /* 1034: 0x8040 */
    "GL_LUMINANCE12_EXT",                              /* 1035: 0x8041 */
    "GL_LUMINANCE16_EXT",                              /* 1036: 0x8042 */
    "GL_LUMINANCE4_ALPHA4_EXT",                        /* 1037: 0x8043 */
    "GL_LUMINANCE6_ALPHA2_EXT",                        /* 1038: 0x8044 */
    "GL_LUMINANCE8_ALPHA8_EXT",                        /* 1039: 0x8045 */
    "GL_LUMINANCE12_ALPHA4_EXT",                       /* 1040: 0x8046 */
    "GL_LUMINANCE12_ALPHA12_EXT",                      /* 1041: 0x8047 */
    "GL_LUMINANCE16_ALPHA16_EXT",                      /* 1042: 0x8048 */
    "GL_INTENSITY_EXT",                                /* 1043: 0x8049 */
    "GL_INTENSITY4_EXT",                               /* 1044: 0x804a */
    "GL_INTENSITY8_EXT",                               /* 1045: 0x804b */
    "GL_INTENSITY12_EXT",                              /* 1046: 0x804c */
    "GL_INTENSITY16_EXT",                              /* 1047: 0x804d */
    "GL_RGB2_EXT",                                     /* 1048: 0x804e */
    "GL_RGB4_EXT",                                     /* 1049: 0x804f */
    "GL_RGB5_EXT",                                     /* 1050: 0x8050 */
    "GL_RGB8_EXT",                                     /* 1051: 0x8051 */
    "GL_RGB10_EXT",                                    /* 1052: 0x8052 */
    "GL_RGB12_EXT",                                    /* 1053: 0x8053 */
    "GL_RGB16_EXT",                                    /* 1054: 0x8054 */
    "GL_RGBA2_EXT",                                    /* 1055: 0x8055 */
    "GL_RGBA4_EXT",                                    /* 1056: 0x8056 */
    "GL_RGB5_A1_EXT",                                  /* 1057: 0x8057 */
    "GL_RGBA8_EXT",                                    /* 1058: 0x8058 */
    "GL_RGB10_A2_EXT",                                 /* 1059: 0x8059 */
    "GL_RGBA12_EXT",                                   /* 1060: 0x805a */
    "GL_RGBA16_EXT",                                   /* 1061: 0x805b */
    "GL_TEXTURE_RED_SIZE_EXT",                         /* 1062: 0x805c */
    "GL_TEXTURE_GREEN_SIZE_EXT",                       /* 1063: 0x805d */
    "GL_TEXTURE_BLUE_SIZE_EXT",                        /* 1064: 0x805e */
    "GL_TEXTURE_ALPHA_SIZE_EXT",                       /* 1065: 0x805f */
    "GL_TEXTURE_LUMINANCE_SIZE_EXT",                   /* 1066: 0x8060 */
    "GL_TEXTURE_INTENSITY_SIZE_EXT",                   /* 1067: 0x8061 */
    "GL_REPLACE_EXT",                                  /* 1068: 0x8062 */
    "GL_PROXY_TEXTURE_1D_EXT",                         /* 1069: 0x8063 */
    "GL_PROXY_TEXTURE_2D_EXT",                         /* 1070: 0x8064 */
    "GL_TEXTURE_TOO_LARGE_EXT",                        /* 1071: 0x8065 */
    "GL_TEXTURE_PRIORITY_EXT",                         /* 1072: 0x8066 */
    "GL_TEXTURE_RESIDENT_EXT",                         /* 1073: 0x8067 */
    "GL_TEXTURE_1D_BINDING_EXT",                       /* 1074: 0x8068 */
    "GL_TEXTURE_2D_BINDING_EXT",                       /* 1075: 0x8069 */
    "GL_TEXTURE_3D_BINDING_EXT",                       /* 1076: 0x806a */
    "GL_PACK_SKIP_IMAGES_EXT",                         /* 1077: 0x806b */
    "GL_PACK_IMAGE_HEIGHT_EXT",                        /* 1078: 0x806c */
    "GL_UNPACK_SKIP_IMAGES_EXT",                       /* 1079: 0x806d */
    "GL_UNPACK_IMAGE_HEIGHT_EXT",                      /* 1080: 0x806e */
    "GL_TEXTURE_3D_EXT",                               /* 1081: 0x806f */
    "GL_PROXY_TEXTURE_3D_EXT",                         /* 1082: 0x8070 */
    "GL_TEXTURE_DEPTH_EXT",                            /* 1083: 0x8071 */
    "GL_TEXTURE_WRAP_R_EXT",                           /* 1084: 0x8072 */
    "GL_MAX_3D_TEXTURE_SIZE_EXT",                      /* 1085: 0x8073 */
    "GL_VERTEX_ARRAY_EXT",                             /* 1086: 0x8074 */
    "GL_NORMAL_ARRAY_EXT",                             /* 1087: 0x8075 */
    "GL_COLOR_ARRAY_EXT",                              /* 1088: 0x8076 */
    "GL_INDEX_ARRAY_EXT",                              /* 1089: 0x8077 */
    "GL_TEXTURE_COORD_ARRAY_EXT",                      /* 1090: 0x8078 */
    "GL_EDGE_FLAG_ARRAY_EXT",                          /* 1091: 0x8079 */
    "GL_VERTEX_ARRAY_SIZE_EXT",                        /* 1092: 0x807a */
    "GL_VERTEX_ARRAY_TYPE_EXT",                        /* 1093: 0x807b */
    "GL_VERTEX_ARRAY_STRIDE_EXT",                      /* 1094: 0x807c */
    "GL_VERTEX_ARRAY_COUNT_EXT",                       /* 1095: 0x807d */
    "GL_NORMAL_ARRAY_TYPE_EXT",                        /* 1096: 0x807e */
    "GL_NORMAL_ARRAY_STRIDE_EXT",                      /* 1097: 0x807f */
    "GL_NORMAL_ARRAY_COUNT_EXT",                       /* 1098: 0x8080 */
    "GL_COLOR_ARRAY_SIZE_EXT",                         /* 1099: 0x8081 */
    "GL_COLOR_ARRAY_TYPE_EXT",                         /* 1100: 0x8082 */
    "GL_COLOR_ARRAY_STRIDE_EXT",                       /* 1101: 0x8083 */
    "GL_COLOR_ARRAY_COUNT_EXT",                        /* 1102: 0x8084 */
    "GL_INDEX_ARRAY_TYPE_EXT",                         /* 1103: 0x8085 */
    "GL_INDEX_ARRAY_STRIDE_EXT",                       /* 1104: 0x8086 */
    "GL_INDEX_ARRAY_COUNT_EXT",                        /* 1105: 0x8087 */
    "GL_TEXTURE_COORD_ARRAY_SIZE_EXT",                 /* 1106: 0x8088 */
    "GL_TEXTURE_COORD_ARRAY_TYPE_EXT",                 /* 1107: 0x8089 */
    "GL_TEXTURE_COORD_ARRAY_STRIDE_EXT",               /* 1108: 0x808a */
    "GL_TEXTURE_COORD_ARRAY_COUNT_EXT",                /* 1109: 0x808b */
    "GL_EDGE_FLAG_ARRAY_STRIDE_EXT",                   /* 1110: 0x808c */
    "GL_EDGE_FLAG_ARRAY_COUNT_EXT",                    /* 1111: 0x808d */
    "GL_VERTEX_ARRAY_POINTER_EXT",                     /* 1112: 0x808e */
    "GL_NORMAL_ARRAY_POINTER_EXT",                     /* 1113: 0x808f */
    "GL_COLOR_ARRAY_POINTER_EXT",                      /* 1114: 0x8090 */
    "GL_INDEX_ARRAY_POINTER_EXT",                      /* 1115: 0x8091 */
    "GL_TEXTURE_COORD_ARRAY_POINTER_EXT",              /* 1116: 0x8092 */
    "GL_EDGE_FLAG_ARRAY_POINTER_EXT",                  /* 1117: 0x8093 */
    "GL_INTERLACE_SGIX",                               /* 1118: 0x8094 */
    "GL_DETAIL_TEXTURE_2D_SGIS",                       /* 1119: 0x8095 */
    "GL_DETAIL_TEXTURE_2D_BINDING_SGIS",               /* 1120: 0x8096 */
    "GL_LINEAR_DETAIL_SGIS",                           /* 1121: 0x8097 */
    "GL_LINEAR_DETAIL_ALPHA_SGIS",                     /* 1122: 0x8098 */
    "GL_LINEAR_DETAIL_COLOR_SGIS",                     /* 1123: 0x8099 */
    "GL_DETAIL_TEXTURE_LEVEL_SGIS",                    /* 1124: 0x809a */
    "GL_DETAIL_TEXTURE_MODE_SGIS",                     /* 1125: 0x809b */
    "GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS",              /* 1126: 0x809c */
    "GL_MULTISAMPLE_SGIS",                             /* 1127: 0x809d */
    "GL_SAMPLE_ALPHA_TO_MASK_SGIS",                    /* 1128: 0x809e */
    "GL_SAMPLE_ALPHA_TO_ONE_SGIS",                     /* 1129: 0x809f */
    "GL_SAMPLE_MASK_SGIS",                             /* 1130: 0x80a0 */
    "GL_1PASS_SGIS",                                   /* 1131: 0x80a1 */
    "GL_2PASS_0_SGIS",                                 /* 1132: 0x80a2 */
    "GL_2PASS_1_SGIS",                                 /* 1133: 0x80a3 */
    "GL_4PASS_0_SGIS",                                 /* 1134: 0x80a4 */
    "GL_4PASS_1_SGIS",                                 /* 1135: 0x80a5 */
    "GL_4PASS_2_SGIS",                                 /* 1136: 0x80a6 */
    "GL_4PASS_3_SGIS",                                 /* 1137: 0x80a7 */
    "GL_SAMPLE_BUFFERS_SGIS",                          /* 1138: 0x80a8 */
    "GL_SAMPLES_SGIS",                                 /* 1139: 0x80a9 */
    "GL_SAMPLE_MASK_VALUE_SGIS",                       /* 1140: 0x80aa */
    "GL_SAMPLE_MASK_INVERT_SGIS",                      /* 1141: 0x80ab */
    "GL_SAMPLE_PATTERN_SGIS",                          /* 1142: 0x80ac */
    "GL_LINEAR_SHARPEN_SGIS",                          /* 1143: 0x80ad */
    "GL_LINEAR_SHARPEN_ALPHA_SGIS",                    /* 1144: 0x80ae */
    "GL_LINEAR_SHARPEN_COLOR_SGIS",                    /* 1145: 0x80af */
    "GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS",             /* 1146: 0x80b0 */
    "GL_COLOR_MATRIX_SGI",                             /* 1147: 0x80b1 */
    "GL_COLOR_MATRIX_STACK_DEPTH_SGI",                 /* 1148: 0x80b2 */
    "GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI",             /* 1149: 0x80b3 */
    "GL_POST_COLOR_MATRIX_RED_SCALE_SGI",              /* 1150: 0x80b4 */
    "GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI",            /* 1151: 0x80b5 */
    "GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI",             /* 1152: 0x80b6 */
    "GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI",            /* 1153: 0x80b7 */
    "GL_POST_COLOR_MATRIX_RED_BIAS_SGI",               /* 1154: 0x80b8 */
    "GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI",             /* 1155: 0x80b9 */
    "GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI",              /* 1156: 0x80ba */
    "GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI",             /* 1157: 0x80bb */
    "GL_TEXTURE_COLOR_TABLE_SGI",                      /* 1158: 0x80bc */
    "GL_PROXY_TEXTURE_COLOR_TABLE_SGI",                /* 1159: 0x80bd */
    "GL_TEXTURE_ENV_BIAS_SGIX",                        /* 1160: 0x80be */
    "GL_SHADOW_AMBIENT_SGIX",                          /* 1161: 0x80bf */
    NULL,                                              /* 1162: */
    NULL,                                              /* 1163: */
    NULL,                                              /* 1164: */
    NULL,                                              /* 1165: */
    NULL,                                              /* 1166: */
    NULL,                                              /* 1167: */
    NULL,                                              /* 1168: */
    NULL,                                              /* 1169: */
    NULL,                                              /* 1170: */
    NULL,                                              /* 1171: */
    NULL,                                              /* 1172: */
    NULL,                                              /* 1173: */
    NULL,                                              /* 1174: */
    NULL,                                              /* 1175: */
    NULL,                                              /* 1176: */
    NULL,                                              /* 1177: */
    "GL_COLOR_TABLE_SGI",                              /* 1178: 0x80d0 */
    "GL_POST_CONVOLUTION_COLOR_TABLE_SGI",             /* 1179: 0x80d1 */
    "GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI",            /* 1180: 0x80d2 */
    "GL_PROXY_COLOR_TABLE_SGI",                        /* 1181: 0x80d3 */
    "GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI",       /* 1182: 0x80d4 */
    "GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI",      /* 1183: 0x80d5 */
    "GL_COLOR_TABLE_SCALE_SGI",                        /* 1184: 0x80d6 */
    "GL_COLOR_TABLE_BIAS_SGI",                         /* 1185: 0x80d7 */
    "GL_COLOR_TABLE_FORMAT_SGI",                       /* 1186: 0x80d8 */
    "GL_COLOR_TABLE_WIDTH_SGI",                        /* 1187: 0x80d9 */
    "GL_COLOR_TABLE_RED_SIZE_SGI",                     /* 1188: 0x80da */
    "GL_COLOR_TABLE_GREEN_SIZE_SGI",                   /* 1189: 0x80db */
    "GL_COLOR_TABLE_BLUE_SIZE_SGI",                    /* 1190: 0x80dc */
    "GL_COLOR_TABLE_ALPHA_SIZE_SGI",                   /* 1191: 0x80dd */
    "GL_COLOR_TABLE_LUMINANCE_SIZE_SGI",               /* 1192: 0x80de */
    "GL_COLOR_TABLE_INTENSITY_SIZE_SGI",               /* 1193: 0x80df */
    NULL,                                              /* 1194: */
    NULL,                                              /* 1195: */
    NULL,                                              /* 1196: */
    NULL,                                              /* 1197: */
    NULL,                                              /* 1198: */
    NULL,                                              /* 1199: */
    NULL,                                              /* 1200: */
    NULL,                                              /* 1201: */
    NULL,                                              /* 1202: */
    NULL,                                              /* 1203: */
    NULL,                                              /* 1204: */
    NULL,                                              /* 1205: */
    NULL,                                              /* 1206: */
    NULL,                                              /* 1207: */
    NULL,                                              /* 1208: */
    NULL,                                              /* 1209: */
    "GL_DUAL_ALPHA4_SGIS",                             /* 1210: 0x8110 */
    "GL_DUAL_ALPHA8_SGIS",                             /* 1211: 0x8111 */
    "GL_DUAL_ALPHA12_SGIS",                            /* 1212: 0x8112 */
    "GL_DUAL_ALPHA16_SGIS",                            /* 1213: 0x8113 */
    "GL_DUAL_LUMINANCE4_SGIS",                         /* 1214: 0x8114 */
    "GL_DUAL_LUMINANCE8_SGIS",                         /* 1215: 0x8115 */
    "GL_DUAL_LUMINANCE12_SGIS",                        /* 1216: 0x8116 */
    "GL_DUAL_LUMINANCE16_SGIS",                        /* 1217: 0x8117 */
    "GL_DUAL_INTENSITY4_SGIS",                         /* 1218: 0x8118 */
    "GL_DUAL_INTENSITY8_SGIS",                         /* 1219: 0x8119 */
    "GL_DUAL_INTENSITY12_SGIS",                        /* 1220: 0x811a */
    "GL_DUAL_INTENSITY16_SGIS",                        /* 1221: 0x811b */
    "GL_DUAL_LUMINANCE_ALPHA4_SGIS",                   /* 1222: 0x811c */
    "GL_DUAL_LUMINANCE_ALPHA8_SGIS",                   /* 1223: 0x811d */
    "GL_QUAD_ALPHA4_SGIS",                             /* 1224: 0x811e */
    "GL_QUAD_ALPHA8_SGIS",                             /* 1225: 0x811f */
    "GL_QUAD_LUMINANCE4_SGIS",                         /* 1226: 0x8120 */
    "GL_QUAD_LUMINANCE8_SGIS",                         /* 1227: 0x8121 */
    "GL_QUAD_INTENSITY4_SGIS",                         /* 1228: 0x8122 */
    "GL_QUAD_INTENSITY8_SGIS",                         /* 1229: 0x8123 */
    "GL_DUAL_TEXTURE_SELECT_SGIS",                     /* 1230: 0x8124 */
    "GL_QUAD_TEXTURE_SELECT_SGIS",                     /* 1231: 0x8125 */
    "GL_POINT_SIZE_MIN_SGIS",                          /* 1232: 0x8126 */
    "GL_POINT_SIZE_MAX_SGIS",                          /* 1233: 0x8127 */
    "GL_POINT_FADE_THRESHOLD_SIZE_SGIS",               /* 1234: 0x8128 */
    "GL_DISTANCE_ATTENUATION_SGIS",                    /* 1235: 0x8129 */
    "GL_FOG_FUNC_SGIS",                                /* 1236: 0x812a */
    "GL_FOG_FUNC_POINTS_SGIS",                         /* 1237: 0x812b */
    "GL_MAX_FOG_FUNC_POINTS_SGIS",                     /* 1238: 0x812c */
    "GL_CLAMP_TO_BORDER_SGIS",                         /* 1239: 0x812d */
    "GL_TEXTURE_MULTI_BUFFER_HINT_SGIX",               /* 1240: 0x812e */
    "GL_CLAMP_TO_EDGE_SGIS",                           /* 1241: 0x812f */
    "GL_PACK_SKIP_VOLUMES_SGIS",                       /* 1242: 0x8130 */
    "GL_PACK_IMAGE_DEPTH_SGIS",                        /* 1243: 0x8131 */
    "GL_UNPACK_SKIP_VOLUMES_SGIS",                     /* 1244: 0x8132 */
    "GL_UNPACK_IMAGE_DEPTH_SGIS",                      /* 1245: 0x8133 */
    "GL_TEXTURE_4D_SGIS",                              /* 1246: 0x8134 */
    "GL_PROXY_TEXTURE_4D_SGIS",                        /* 1247: 0x8135 */
    "GL_TEXTURE_4DSIZE_SGIS",                          /* 1248: 0x8136 */
    "GL_TEXTURE_WRAP_Q_SGIS",                          /* 1249: 0x8137 */
    "GL_MAX_4D_TEXTURE_SIZE_SGIS",                     /* 1250: 0x8138 */
    "GL_PIXEL_TEX_GEN_SGIX",                           /* 1251: 0x8139 */
    "GL_TEXTURE_MIN_LOD_SGIS",                         /* 1252: 0x813a */
    "GL_TEXTURE_MAX_LOD_SGIS",                         /* 1253: 0x813b */
    "GL_TEXTURE_BASE_LEVEL_SGIS",                      /* 1254: 0x813c */
    "GL_TEXTURE_MAX_LEVEL_SGIS",                       /* 1255: 0x813d */
    "GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX",               /* 1256: 0x813e */
    "GL_PIXEL_TILE_CACHE_INCREMENT_SGIX",              /* 1257: 0x813f */
    "GL_PIXEL_TILE_WIDTH_SGIX",                        /* 1258: 0x8140 */
    "GL_PIXEL_TILE_HEIGHT_SGIX",                       /* 1259: 0x8141 */
    "GL_PIXEL_TILE_GRID_WIDTH_SGIX",                   /* 1260: 0x8142 */
    "GL_PIXEL_TILE_GRID_HEIGHT_SGIX",                  /* 1261: 0x8143 */
    "GL_PIXEL_TILE_GRID_DEPTH_SGIX",                   /* 1262: 0x8144 */
    "GL_PIXEL_TILE_CACHE_SIZE_SGIX",                   /* 1263: 0x8145 */
    "GL_FILTER4_SGIS",                                 /* 1264: 0x8146 */
    "GL_TEXTURE_FILTER4_SIZE_SGIS",                    /* 1265: 0x8147 */
    "GL_SPRITE_SGIX",                                  /* 1266: 0x8148 */
    "GL_SPRITE_MODE_SGIX",                             /* 1267: 0x8149 */
    "GL_SPRITE_AXIS_SGIX",                             /* 1268: 0x814a */
    "GL_SPRITE_TRANSLATION_SGIX",                      /* 1269: 0x814b */
    "GL_SPRITE_AXIAL_SGIX",                            /* 1270: 0x814c */
    "GL_SPRITE_OBJECT_ALIGNED_SGIX",                   /* 1271: 0x814d */
    "GL_SPRITE_EYE_ALIGNED_SGIX",                      /* 1272: 0x814e */
    "GL_TEXTURE_4D_BINDING_SGIS",                      /* 1273: 0x814f */
    NULL,                                              /* 1274: */
    NULL,                                              /* 1275: */
    NULL,                                              /* 1276: */
    NULL,                                              /* 1277: */
    NULL,                                              /* 1278: */
    NULL,                                              /* 1279: */
    NULL,                                              /* 1280: */
    NULL,                                              /* 1281: */
    NULL,                                              /* 1282: */
    NULL,                                              /* 1283: */
    NULL,                                              /* 1284: */
    NULL,                                              /* 1285: */
    NULL,                                              /* 1286: */
    NULL,                                              /* 1287: */
    NULL,                                              /* 1288: */
    NULL,                                              /* 1289: */
    NULL,                                              /* 1290: */
    NULL,                                              /* 1291: */
    NULL,                                              /* 1292: */
    NULL,                                              /* 1293: */
    NULL,                                              /* 1294: */
    NULL,                                              /* 1295: */
    NULL,                                              /* 1296: */
    NULL,                                              /* 1297: */
    NULL,                                              /* 1298: */
    NULL,                                              /* 1299: */
    NULL,                                              /* 1300: */
    NULL,                                              /* 1301: */
    NULL,                                              /* 1302: */
    NULL,                                              /* 1303: */
    NULL,                                              /* 1304: */
    NULL,                                              /* 1305: */
    "GL_LINEAR_CLIPMAP_LINEAR_SGIX",                   /* 1306: 0x8170 */
    "GL_TEXTURE_CLIPMAP_CENTER_SGIX",                  /* 1307: 0x8171 */
    "GL_TEXTURE_CLIPMAP_FRAME_SGIX",                   /* 1308: 0x8172 */
    "GL_TEXTURE_CLIPMAP_OFFSET_SGIX",                  /* 1309: 0x8173 */
    "GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX",           /* 1310: 0x8174 */
    "GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX",              /* 1311: 0x8175 */
    "GL_TEXTURE_CLIPMAP_DEPTH_SGIX",                   /* 1312: 0x8176 */
    "GL_MAX_CLIPMAP_DEPTH_SGIX",                       /* 1313: 0x8177 */
    "GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX",               /* 1314: 0x8178 */
    "GL_POST_TEXTURE_FILTER_BIAS_SGIX",                /* 1315: 0x8179 */
    "GL_POST_TEXTURE_FILTER_SCALE_SGIX",               /* 1316: 0x817a */
    "GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX",          /* 1317: 0x817b */
    "GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX",         /* 1318: 0x817c */
    "GL_REFERENCE_PLANE_SGIX",                         /* 1319: 0x817d */
    "GL_REFERENCE_PLANE_EQUATION_SGIX",                /* 1320: 0x817e */
    "GL_IR_INSTRUMENT1_SGIX",                          /* 1321: 0x817f */
    "GL_INSTRUMENT_BUFFER_POINTER_SGIX",               /* 1322: 0x8180 */
    "GL_INSTRUMENT_MEASUREMENTS_SGIX",                 /* 1323: 0x8181 */
    "GL_LIST_PRIORITY_SGIX",                           /* 1324: 0x8182 */
    "GL_CALLIGRAPHIC_FRAGMENT_SGIX",                   /* 1325: 0x8183 */
    "GL_PIXEL_TEX_GEN_Q_CEILING_SGIX",                 /* 1326: 0x8184 */
    "GL_PIXEL_TEX_GEN_Q_ROUND_SGIX",                   /* 1327: 0x8185 */
    "GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX",                   /* 1328: 0x8186 */
    "GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX",             /* 1329: 0x8187 */
    "GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX",          /* 1330: 0x8188 */
    "GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX",                  /* 1331: 0x8189 */
    "GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX",                  /* 1332: 0x818a */
    "GL_FRAMEZOOM_SGIX",                               /* 1333: 0x818b */
    "GL_FRAMEZOOM_FACTOR_SGIX",                        /* 1334: 0x818c */
    "GL_MAX_FRAMEZOOM_FACTOR_SGIX",                    /* 1335: 0x818d */
    "GL_TEXTURE_LOD_BIAS_S_SGIX",                      /* 1336: 0x818e */
    "GL_TEXTURE_LOD_BIAS_T_SGIX",                      /* 1337: 0x818f */
    "GL_TEXTURE_LOD_BIAS_R_SGIX",                      /* 1338: 0x8190 */
    "GL_GENERATE_MIPMAP_SGIS",                         /* 1339: 0x8191 */
    "GL_GENERATE_MIPMAP_HINT_SGIS",                    /* 1340: 0x8192 */
    NULL,                                              /* 1341: */
    "GL_GEOMETRY_DEFORMATION_SGIX",                    /* 1342: 0x8194 */
    "GL_TEXTURE_DEFORMATION_SGIX",                     /* 1343: 0x8195 */
    "GL_DEFORMATIONS_MASK_SGIX",                       /* 1344: 0x8196 */
    "GL_MAX_DEFORMATION_ORDER_SGIX",                   /* 1345: 0x8197 */
    "GL_FOG_OFFSET_SGIX",                              /* 1346: 0x8198 */
    "GL_FOG_OFFSET_VALUE_SGIX",                        /* 1347: 0x8199 */
    "GL_TEXTURE_COMPARE_SGIX",                         /* 1348: 0x819a */
    "GL_TEXTURE_COMPARE_OPERATOR_SGIX",                /* 1349: 0x819b */
    "GL_TEXTURE_LEQUAL_R_SGIX",                        /* 1350: 0x819c */
    "GL_TEXTURE_GEQUAL_R_SGIX",                        /* 1351: 0x819d */
    NULL,                                              /* 1352: */
    NULL,                                              /* 1353: */
    NULL,                                              /* 1354: */
    NULL,                                              /* 1355: */
    NULL,                                              /* 1356: */
    NULL,                                              /* 1357: */
    NULL,                                              /* 1358: */
    "GL_DEPTH_COMPONENT16_SGIX",                       /* 1359: 0x81a5 */
    "GL_DEPTH_COMPONENT24_SGIX",                       /* 1360: 0x81a6 */
    "GL_DEPTH_COMPONENT32_SGIX",                       /* 1361: 0x81a7 */
    NULL,                                              /* 1362: */
    NULL,                                              /* 1363: */
    NULL,                                              /* 1364: */
    NULL,                                              /* 1365: */
    NULL,                                              /* 1366: */
    NULL,                                              /* 1367: */
    NULL,                                              /* 1368: */
    NULL,                                              /* 1369: */
    NULL,                                              /* 1370: */
    NULL,                                              /* 1371: */
    NULL,                                              /* 1372: */
    NULL,                                              /* 1373: */
    NULL,                                              /* 1374: */
    NULL,                                              /* 1375: */
    NULL,                                              /* 1376: */
    NULL,                                              /* 1377: */
    NULL,                                              /* 1378: */
    NULL,                                              /* 1379: */
    NULL,                                              /* 1380: */
    "GL_YCRCB_422_SGIX",                               /* 1381: 0x81bb */
    "GL_YCRCB_444_SGIX",                               /* 1382: 0x81bc */
    NULL,                                              /* 1383: */
    NULL,                                              /* 1384: */
    NULL,                                              /* 1385: */
    NULL,                                              /* 1386: */
    NULL,                                              /* 1387: */
    NULL,                                              /* 1388: */
    NULL,                                              /* 1389: */
    NULL,                                              /* 1390: */
    NULL,                                              /* 1391: */
    NULL,                                              /* 1392: */
    NULL,                                              /* 1393: */
    NULL,                                              /* 1394: */
    NULL,                                              /* 1395: */
    NULL,                                              /* 1396: */
    NULL,                                              /* 1397: */
    NULL,                                              /* 1398: */
    NULL,                                              /* 1399: */
    NULL,                                              /* 1400: */
    NULL,                                              /* 1401: */
    NULL,                                              /* 1402: */
    NULL,                                              /* 1403: */
    NULL,                                              /* 1404: */
    NULL,                                              /* 1405: */
    NULL,                                              /* 1406: */
    NULL,                                              /* 1407: */
    NULL,                                              /* 1408: */
    NULL,                                              /* 1409: */
    NULL,                                              /* 1410: */
    NULL,                                              /* 1411: */
    NULL,                                              /* 1412: */
    NULL,                                              /* 1413: */
    NULL,                                              /* 1414: */
    NULL,                                              /* 1415: */
    NULL,                                              /* 1416: */
    NULL,                                              /* 1417: */
    NULL,                                              /* 1418: */
    NULL,                                              /* 1419: */
    NULL,                                              /* 1420: */
    NULL,                                              /* 1421: */
    NULL,                                              /* 1422: */
    NULL,                                              /* 1423: */
    NULL,                                              /* 1424: */
    NULL,                                              /* 1425: */
    NULL,                                              /* 1426: */
    NULL,                                              /* 1427: */
    NULL,                                              /* 1428: */
    NULL,                                              /* 1429: */
    NULL,                                              /* 1430: */
    NULL,                                              /* 1431: */
    NULL,                                              /* 1432: */
    NULL,                                              /* 1433: */
    "GL_EYE_DISTANCE_TO_POINT_SGIS",                   /* 1434: 0x81f0 */
    "GL_OBJECT_DISTANCE_TO_POINT_SGIS",                /* 1435: 0x81f1 */
    "GL_EYE_DISTANCE_TO_LINE_SGIS",                    /* 1436: 0x81f2 */
    "GL_OBJECT_DISTANCE_TO_LINE_SGIS",                 /* 1437: 0x81f3 */
    "GL_EYE_POINT_SGIS",                               /* 1438: 0x81f4 */
    "GL_OBJECT_POINT_SGIS",                            /* 1439: 0x81f5 */
    "GL_EYE_LINE_SGIS",                                /* 1440: 0x81f6 */
    "GL_OBJECT_LINE_SGIS",                             /* 1441: 0x81f7 */
    "GL_POINTS",                                       /* 1442: 0x0000 */
    "GL_LINES",                                        /* 1443: 0x0001 */
    "GL_LINE_LOOP",                                    /* 1444: 0x0002 */
    "GL_LINE_STRIP",                                   /* 1445: 0x0003 */
    "GL_TRIANGLES",                                    /* 1446: 0x0004 */
    "GL_TRIANGLE_STRIP",                               /* 1447: 0x0005 */
    "GL_TRIANGLE_FAN",                                 /* 1448: 0x0006 */
    "GL_QUADS",                                        /* 1449: 0x0007 */
    "GL_QUAD_STRIP",                                   /* 1450: 0x0008 */
    "GL_POLYGON",                                      /* 1451: 0x0009 */
    NULL,                                              /* 1452: */
    NULL,                                              /* 1453: */
    NULL,                                              /* 1454: */
    NULL,                                              /* 1455: */
    NULL,                                              /* 1456: */
    NULL,                                              /* 1457: */
    NULL,                                              /* 1458: */
    NULL,                                              /* 1459: */
    NULL,                                              /* 1460: */
    NULL,                                              /* 1461: */
    NULL,                                              /* 1462: */
    NULL,                                              /* 1463: */
    NULL,                                              /* 1464: */
    NULL,                                              /* 1465: */
    NULL,                                              /* 1466: */
    NULL,                                              /* 1467: */
    NULL,                                              /* 1468: */
    NULL,                                              /* 1469: */
    NULL,                                              /* 1470: */
    NULL,                                              /* 1471: */
    NULL,                                              /* 1472: */
    NULL,                                              /* 1473: */
    NULL,                                              /* 1474: */
    NULL,                                              /* 1475: */
    NULL,                                              /* 1476: */
    NULL,                                              /* 1477: */
    NULL,                                              /* 1478: */
    NULL,                                              /* 1479: */
    NULL,                                              /* 1480: */
    NULL,                                              /* 1481: */
    NULL,                                              /* 1482: */
    NULL,                                              /* 1483: */
    NULL,                                              /* 1484: */
    NULL,                                              /* 1485: */
    NULL,                                              /* 1486: */
    NULL,                                              /* 1487: */
    NULL,                                              /* 1488: */
    NULL,                                              /* 1489: */
    NULL,                                              /* 1490: */
    NULL,                                              /* 1491: */
    NULL,                                              /* 1492: */
    NULL,                                              /* 1493: */
    NULL,                                              /* 1494: */
    NULL,                                              /* 1495: */
    NULL,                                              /* 1496: */
    NULL,                                              /* 1497: */
    NULL,                                              /* 1498: */
    NULL,                                              /* 1499: */
    NULL,                                              /* 1500: */
    NULL,                                              /* 1501: */
    NULL,                                              /* 1502: */
    NULL,                                              /* 1503: */
    NULL,                                              /* 1504: */
    NULL,                                              /* 1505: */
    NULL,                                              /* 1506: */
    NULL,                                              /* 1507: */
    NULL,                                              /* 1508: */
    NULL,                                              /* 1509: */
    NULL,                                              /* 1510: */
    NULL,                                              /* 1511: */
    NULL,                                              /* 1512: */
    NULL,                                              /* 1513: */
    NULL,                                              /* 1514: */
    NULL,                                              /* 1515: */
    NULL,                                              /* 1516: */
    NULL,                                              /* 1517: */
    NULL,                                              /* 1518: */
    NULL,                                              /* 1519: */
    NULL,                                              /* 1520: */
    NULL,                                              /* 1521: */
    NULL,                                              /* 1522: */
    NULL,                                              /* 1523: */
    NULL,                                              /* 1524: */
    NULL,                                              /* 1525: */
    NULL,                                              /* 1526: */
    NULL,                                              /* 1527: */
    NULL,                                              /* 1528: */
    NULL,                                              /* 1529: */
    NULL,                                              /* 1530: */
    NULL,                                              /* 1531: */
    NULL,                                              /* 1532: */
    NULL,                                              /* 1533: */
    NULL,                                              /* 1534: */
    NULL,                                              /* 1535: */
    NULL,                                              /* 1536: */
    NULL,                                              /* 1537: */
    NULL,                                              /* 1538: */
    NULL,                                              /* 1539: */
    NULL,                                              /* 1540: */
    NULL,                                              /* 1541: */
    NULL,                                              /* 1542: */
    NULL,                                              /* 1543: */
    NULL,                                              /* 1544: */
    NULL,                                              /* 1545: */
    NULL,                                              /* 1546: */
    NULL,                                              /* 1547: */
    NULL,                                              /* 1548: */
    NULL,                                              /* 1549: */
    NULL,                                              /* 1550: */
    NULL,                                              /* 1551: */
    NULL,                                              /* 1552: */
    NULL,                                              /* 1553: */
    NULL,                                              /* 1554: */
    NULL,                                              /* 1555: */
    NULL,                                              /* 1556: */
    NULL,                                              /* 1557: */
    NULL,                                              /* 1558: */
    NULL,                                              /* 1559: */
    NULL,                                              /* 1560: */
    NULL,                                              /* 1561: */
    NULL,                                              /* 1562: */
    NULL,                                              /* 1563: */
    NULL,                                              /* 1564: */
    NULL,                                              /* 1565: */
    NULL,                                              /* 1566: */
    NULL,                                              /* 1567: */
    NULL,                                              /* 1568: */
    NULL,                                              /* 1569: */
    NULL,                                              /* 1570: */
    NULL,                                              /* 1571: */
    NULL,                                              /* 1572: */
    NULL,                                              /* 1573: */
    NULL,                                              /* 1574: */
    NULL,                                              /* 1575: */
    NULL,                                              /* 1576: */
    NULL,                                              /* 1577: */
    NULL,                                              /* 1578: */
    NULL,                                              /* 1579: */
    NULL,                                              /* 1580: */
    NULL,                                              /* 1581: */
    NULL,                                              /* 1582: */
    NULL,                                              /* 1583: */
    NULL,                                              /* 1584: */
    NULL,                                              /* 1585: */
    NULL,                                              /* 1586: */
    NULL,                                              /* 1587: */
    NULL,                                              /* 1588: */
    NULL,                                              /* 1589: */
    NULL,                                              /* 1590: */
    NULL,                                              /* 1591: */
    NULL,                                              /* 1592: */
    NULL,                                              /* 1593: */
    NULL,                                              /* 1594: */
    NULL,                                              /* 1595: */
    NULL,                                              /* 1596: */
    NULL,                                              /* 1597: */
    NULL,                                              /* 1598: */
    NULL,                                              /* 1599: */
    NULL,                                              /* 1600: */
    NULL,                                              /* 1601: */
    NULL,                                              /* 1602: */
    NULL,                                              /* 1603: */
    NULL,                                              /* 1604: */
    NULL,                                              /* 1605: */
    NULL,                                              /* 1606: */
    NULL,                                              /* 1607: */
    NULL,                                              /* 1608: */
    NULL,                                              /* 1609: */
    NULL,                                              /* 1610: */
    NULL,                                              /* 1611: */
    NULL,                                              /* 1612: */
    NULL,                                              /* 1613: */
    NULL,                                              /* 1614: */
    NULL,                                              /* 1615: */
    NULL,                                              /* 1616: */
    NULL,                                              /* 1617: */
    NULL,                                              /* 1618: */
    NULL,                                              /* 1619: */
    NULL,                                              /* 1620: */
    NULL,                                              /* 1621: */
    NULL,                                              /* 1622: */
    NULL,                                              /* 1623: */
    NULL,                                              /* 1624: */
    NULL,                                              /* 1625: */
    NULL,                                              /* 1626: */
    NULL,                                              /* 1627: */
    NULL,                                              /* 1628: */
    NULL,                                              /* 1629: */
    NULL,                                              /* 1630: */
    NULL,                                              /* 1631: */
    NULL,                                              /* 1632: */
    NULL,                                              /* 1633: */
    NULL,                                              /* 1634: */
    NULL,                                              /* 1635: */
    NULL,                                              /* 1636: */
    NULL,                                              /* 1637: */
    NULL,                                              /* 1638: */
    NULL,                                              /* 1639: */
    NULL,                                              /* 1640: */
    NULL,                                              /* 1641: */
    NULL,                                              /* 1642: */
    NULL,                                              /* 1643: */
    NULL,                                              /* 1644: */
    NULL,                                              /* 1645: */
    NULL,                                              /* 1646: */
    NULL,                                              /* 1647: */
    NULL,                                              /* 1648: */
    NULL,                                              /* 1649: */
    NULL,                                              /* 1650: */
    NULL,                                              /* 1651: */
    NULL,                                              /* 1652: */
    NULL,                                              /* 1653: */
    NULL,                                              /* 1654: */
    NULL,                                              /* 1655: */
    NULL,                                              /* 1656: */
    NULL,                                              /* 1657: */
    NULL,                                              /* 1658: */
    NULL,                                              /* 1659: */
    NULL,                                              /* 1660: */
    NULL,                                              /* 1661: */
    NULL,                                              /* 1662: */
    NULL,                                              /* 1663: */
    NULL,                                              /* 1664: */
    NULL,                                              /* 1665: */
    NULL,                                              /* 1666: */
    NULL,                                              /* 1667: */
    NULL,                                              /* 1668: */
    NULL,                                              /* 1669: */
    NULL,                                              /* 1670: */
    NULL,                                              /* 1671: */
    NULL,                                              /* 1672: */
    NULL,                                              /* 1673: */
    NULL,                                              /* 1674: */
    NULL,                                              /* 1675: */
    NULL,                                              /* 1676: */
    NULL,                                              /* 1677: */
    NULL,                                              /* 1678: */
    NULL,                                              /* 1679: */
    NULL,                                              /* 1680: */
    NULL,                                              /* 1681: */
    NULL,                                              /* 1682: */
    NULL,                                              /* 1683: */
    NULL,                                              /* 1684: */
    NULL,                                              /* 1685: */
    NULL,                                              /* 1686: */
    NULL,                                              /* 1687: */
    NULL,                                              /* 1688: */
    NULL,                                              /* 1689: */
    NULL,                                              /* 1690: */
    NULL,                                              /* 1691: */
    "GL_CURSOR_POSITION_CR",                           /* 1692: 0x8AF0 */
    "GL_DEFAULT_BBOX_CR",                              /* 1693: 0x8AF1 */
    "GL_SCREEN_BBOX_CR",                               /* 1694: 0x8AF2 */
    "GL_OBJECT_BBOX_CR",                               /* 1695: 0x8AF3 */
    "GL_PRINT_STRING_CR",                              /* 1696: 0x8AF4 */
    "GL_MURAL_SIZE_CR",                                /* 1697: 0x8AF5 */
    "GL_NUM_SERVERS_CR",                               /* 1698: 0x8AF6 */
    "GL_NUM_TILES_CR",                                 /* 1699: 0x8AF7 */
    "GL_TILE_BOUNDS_CR",                               /* 1700: 0x8AF8 */
    "GL_VERTEX_COUNTS_CR",                             /* 1701: 0x8AF9 */
    "GL_RESET_VERTEX_COUNTERS_CR",                     /* 1702: 0x8AFA */
    "GL_SET_MAX_VIEWPORT_CR",                          /* 1703: 0x8AFB */
    "GL_HEAD_SPU_NAME_CR",                             /* 1704: 0x8AFC */
    "GL_PERF_GET_FRAME_DATA_CR",                       /* 1705: 0x8AFD */
    "GL_PERF_GET_TIMER_DATA_CR",                       /* 1706: 0x8AFE */
    "GL_PERF_DUMP_COUNTERS_CR",                        /* 1707: 0x8AFF */
    "GL_PERF_SET_TOKEN_CR",                            /* 1708: 0x8B00 */
    "GL_PERF_SET_DUMP_ON_SWAP_CR",                     /* 1709: 0x8B01 */
    "GL_PERF_SET_DUMP_ON_FINISH_CR",                   /* 1710: 0x8B02 */
    "GL_PERF_SET_DUMP_ON_FLUSH_CR",                    /* 1711: 0x8B03 */
    "GL_PERF_START_TIMER_CR",                          /* 1712: 0x8B04 */
    "GL_PERF_STOP_TIMER_CR",                           /* 1713: 0x8B05 */
    "GL_WINDOW_SIZE_CR",                               /* 1714: 0x8B06 */
    "GL_TILE_INFO_CR",                                 /* 1715: 0x8B07 */
    "GL_GATHER_DRAWPIXELS_CR",                         /* 1716: 0x8B08 */
    "GL_GATHER_PACK_CR",                               /* 1717: 0x8B09 */
    "GL_GATHER_CONNECT_CR",                            /* 1718: 0x8B0A */
    "GL_GATHER_POST_SWAPBUFFERS_CR",                   /* 1719: 0x8B0B */
    "GL_SAVEFRAME_ENABLED_CR",                         /* 1720: 0x8B0C */
    "GL_SAVEFRAME_FRAMENUM_CR",                        /* 1721: 0x8B0D */
    "GL_SAVEFRAME_STRIDE_CR",                          /* 1722: 0x8B0E */
    "GL_SAVEFRAME_SINGLE_CR",                          /* 1723: 0x8B0F */
    "GL_SAVEFRAME_FILESPEC_CR",                        /* 1724: 0x8B10 */
    "GL_READBACK_BARRIER_SIZE_CR",                     /* 1725: 0x8B11 */
};

char * printspuEnumToStr(GLenum value)
{
    static char buf[16];
		if (value < 0x10000) {
					// must be a 2-byte value, so these calculations work
					int msb = value >> 8;
					int lsb = value & 0xff;
					GLT_enum_tab_ent *ent = &__enum_tab_ents[msb];
					if (lsb < ent->count) {
							char *str = __enum_str_tab[ent->index + lsb];
							if (str)
									return str;
					}
			}
    sprintf(buf, "0x%x", (unsigned int) value);
    return buf;
}

