/* Copyright (c) 2001, Stanford University 
 * All rights reserved 
 * 
 * See the file LICENSE.txt for information on redistributing this software.
 * 
 * Stolen from GLT */

#include "chromium.h"
#include "printspu.h"

typedef struct {
    short  index;
    short  count;
} GLT_enum_tab_ent;

static GLT_enum_tab_ent __enum_tab_ents[256] = {
    {1442, 10},{2, 5},{7, 8},{15, 9},{24, 13},{37, 6},{43, 5},{48, 8},{56, 2},
    {58, 2},{60, 3},{63, 243},{306, 246},{552, 245},{-1, -1},{-1, -1},
    {797, 6},{803, 3},{806, 10},{816, 2},{818, 11},{829, 16},{845, 4},
    {849, 3},{852, 3},{855, 11},{866, 1},{867, 3},{870, 3},{873, 2},{875, 4},
    {879, 4},{883, 4},{887, 2},{889, 2},{891, 1},{892, 3},{895, 3},{898, 2},
    {900, 4},{904, 4},{908, 2},{910, 46},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{956, 6},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{962, 8},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{970, 224},{1194, 248},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},
    {-1, -1},{-1, -1}
};

/* String table */

static char *__enum_str_tab[] = {
    "GL_ZERO",                                /* 0x0000 */
    "GL_ONE",                                 /* 0x0001 */
    "GL_ACCUM",                               /* 0x0100 */
    "GL_LOAD",                                /* 0x0101 */
    "GL_RETURN",                              /* 0x0102 */
    "GL_MULT",                                /* 0x0103 */
    "GL_ADD",                                 /* 0x0104 */
    "GL_NEVER",                               /* 0x0200 */
    "GL_LESS",                                /* 0x0201 */
    "GL_EQUAL",                               /* 0x0202 */
    "GL_LEQUAL",                              /* 0x0203 */
    "GL_GREATER",                             /* 0x0204 */
    "GL_NOTEQUAL",                            /* 0x0205 */
    "GL_GEQUAL",                              /* 0x0206 */
    "GL_ALWAYS",                              /* 0x0207 */
    "GL_SRC_COLOR",                           /* 0x0300 */
    "GL_ONE_MINUS_SRC_COLOR",                 /* 0x0301 */
    "GL_SRC_ALPHA",                           /* 0x0302 */
    "GL_ONE_MINUS_SRC_ALPHA",                 /* 0x0303 */
    "GL_DST_ALPHA",                           /* 0x0304 */
    "GL_ONE_MINUS_DST_ALPHA",                 /* 0x0305 */
    "GL_DST_COLOR",                           /* 0x0306 */
    "GL_ONE_MINUS_DST_COLOR",                 /* 0x0307 */
    "GL_SRC_ALPHA_SATURATE",                  /* 0x0308 */
    "GL_FRONT_LEFT",                          /* 0x0400 */
    "GL_FRONT_RIGHT",                         /* 0x0401 */
    "GL_BACK_LEFT",                           /* 0x0402 */
    "GL_BACK_RIGHT",                          /* 0x0403 */
    "GL_FRONT",                               /* 0x0404 */
    "GL_BACK",                                /* 0x0405 */
    "GL_LEFT",                                /* 0x0406 */
    "GL_RIGHT",                               /* 0x0407 */
    "GL_FRONT_AND_BACK",                      /* 0x0408 */
    "GL_AUX0",                                /* 0x0409 */
    "GL_AUX1",                                /* 0x040a */
    "GL_AUX2",                                /* 0x040b */
    "GL_AUX3",                                /* 0x040c */
    "GL_INVALID_ENUM",                        /* 0x0500 */
    "GL_INVALID_VALUE",                       /* 0x0501 */
    "GL_INVALID_OPERATION",                   /* 0x0502 */
    "GL_STACK_OVERFLOW",                      /* 0x0503 */
    "GL_STACK_UNDERFLOW",                     /* 0x0504 */
    "GL_OUT_OF_MEMORY",                       /* 0x0505 */
    "GL_2D",                                  /* 0x0600 */
    "GL_3D",                                  /* 0x0601 */
    "GL_3D_COLOR",                            /* 0x0602 */
    "GL_3D_COLOR_TEXTURE",                    /* 0x0603 */
    "GL_4D_COLOR_TEXTURE",                    /* 0x0604 */
    "GL_PASS_THROUGH_TOKEN",                  /* 0x0700 */
    "GL_POINT_TOKEN",                         /* 0x0701 */
    "GL_LINE_TOKEN",                          /* 0x0702 */
    "GL_POLYGON_TOKEN",                       /* 0x0703 */
    "GL_BITMAP_TOKEN",                        /* 0x0704 */
    "GL_DRAW_PIXEL_TOKEN",                    /* 0x0705 */
    "GL_COPY_PIXEL_TOKEN",                    /* 0x0706 */
    "GL_LINE_RESET_TOKEN",                    /* 0x0707 */
    "GL_EXP",                                 /* 0x0800 */
    "GL_EXP2",                                /* 0x0801 */
    "GL_CW",                                  /* 0x0900 */
    "GL_CCW",                                 /* 0x0901 */
    "GL_COEFF",                               /* 0x0a00 */
    "GL_ORDER",                               /* 0x0a01 */
    "GL_DOMAIN",                              /* 0x0a02 */
    "GL_CURRENT_COLOR",                       /* 0x0b00 */
    "GL_CURRENT_INDEX",                       /* 0x0b01 */
    "GL_CURRENT_NORMAL",                      /* 0x0b02 */
    "GL_CURRENT_TEXTURE_COORDS",              /* 0x0b03 */
    "GL_CURRENT_RASTER_COLOR",                /* 0x0b04 */
    "GL_CURRENT_RASTER_INDEX",                /* 0x0b05 */
    "GL_CURRENT_RASTER_TEXTURE_COORDS",       /* 0x0b06 */
    "GL_CURRENT_RASTER_POSITION",             /* 0x0b07 */
    "GL_CURRENT_RASTER_POSITION_VALID",       /* 0x0b08 */
    "GL_CURRENT_RASTER_DISTANCE",             /* 0x0b09 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_POINT_SMOOTH",                        /* 0x0b10 */
    "GL_POINT_SIZE",                          /* 0x0b11 */
    "GL_POINT_SIZE_RANGE",                    /* 0x0b12 */
    "GL_POINT_SIZE_GRANULARITY",              /* 0x0b13 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_LINE_SMOOTH",                         /* 0x0b20 */
    "GL_LINE_WIDTH",                          /* 0x0b21 */
    "GL_LINE_WIDTH_RANGE",                    /* 0x0b22 */
    "GL_LINE_WIDTH_GRANULARITY",              /* 0x0b23 */
    "GL_LINE_STIPPLE",                        /* 0x0b24 */
    "GL_LINE_STIPPLE_PATTERN",                /* 0x0b25 */
    "GL_LINE_STIPPLE_REPEAT",                 /* 0x0b26 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_LIST_MODE",                           /* 0x0b30 */
    "GL_MAX_LIST_NESTING",                    /* 0x0b31 */
    "GL_LIST_BASE",                           /* 0x0b32 */
    "GL_LIST_INDEX",                          /* 0x0b33 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_POLYGON_MODE",                        /* 0x0b40 */
    "GL_POLYGON_SMOOTH",                      /* 0x0b41 */
    "GL_POLYGON_STIPPLE",                     /* 0x0b42 */
    "GL_EDGE_FLAG",                           /* 0x0b43 */
    "GL_CULL_FACE",                           /* 0x0b44 */
    "GL_CULL_FACE_MODE",                      /* 0x0b45 */
    "GL_FRONT_FACE",                          /* 0x0b46 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_LIGHTING",                            /* 0x0b50 */
    "GL_LIGHT_MODEL_LOCAL_VIEWER",            /* 0x0b51 */
    "GL_LIGHT_MODEL_TWO_SIDE",                /* 0x0b52 */
    "GL_LIGHT_MODEL_AMBIENT",                 /* 0x0b53 */
    "GL_SHADE_MODEL",                         /* 0x0b54 */
    "GL_COLOR_MATERIAL_FACE",                 /* 0x0b55 */
    "GL_COLOR_MATERIAL_PARAMETER",            /* 0x0b56 */
    "GL_COLOR_MATERIAL",                      /* 0x0b57 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_FOG",                                 /* 0x0b60 */
    "GL_FOG_INDEX",                           /* 0x0b61 */
    "GL_FOG_DENSITY",                         /* 0x0b62 */
    "GL_FOG_START",                           /* 0x0b63 */
    "GL_FOG_END",                             /* 0x0b64 */
    "GL_FOG_MODE",                            /* 0x0b65 */
    "GL_FOG_COLOR",                           /* 0x0b66 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_DEPTH_RANGE",                         /* 0x0b70 */
    "GL_DEPTH_TEST",                          /* 0x0b71 */
    "GL_DEPTH_WRITEMASK",                     /* 0x0b72 */
    "GL_DEPTH_CLEAR_VALUE",                   /* 0x0b73 */
    "GL_DEPTH_FUNC",                          /* 0x0b74 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_ACCUM_CLEAR_VALUE",                   /* 0x0b80 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_STENCIL_TEST",                        /* 0x0b90 */
    "GL_STENCIL_CLEAR_VALUE",                 /* 0x0b91 */
    "GL_STENCIL_FUNC",                        /* 0x0b92 */
    "GL_STENCIL_VALUE_MASK",                  /* 0x0b93 */
    "GL_STENCIL_FAIL",                        /* 0x0b94 */
    "GL_STENCIL_PASS_DEPTH_FAIL",             /* 0x0b95 */
    "GL_STENCIL_PASS_DEPTH_PASS",             /* 0x0b96 */
    "GL_STENCIL_REF",                         /* 0x0b97 */
    "GL_STENCIL_WRITEMASK",                   /* 0x0b98 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_MATRIX_MODE",                         /* 0x0ba0 */
    "GL_NORMALIZE",                           /* 0x0ba1 */
    "GL_VIEWPORT",                            /* 0x0ba2 */
    "GL_MODELVIEW_STACK_DEPTH",               /* 0x0ba3 */
    "GL_PROJECTION_STACK_DEPTH",              /* 0x0ba4 */
    "GL_TEXTURE_STACK_DEPTH",                 /* 0x0ba5 */
    "GL_MODELVIEW_MATRIX",                    /* 0x0ba6 */
    "GL_PROJECTION_MATRIX",                   /* 0x0ba7 */
    "GL_TEXTURE_MATRIX",                      /* 0x0ba8 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_ATTRIB_STACK_DEPTH",                  /* 0x0bb0 */
    "GL_CLIENT_ATTRIB_STACK_DEPTH",           /* 0x0bb1 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_ALPHA_TEST",                          /* 0x0bc0 */
    "GL_ALPHA_TEST_FUNC",                     /* 0x0bc1 */
    "GL_ALPHA_TEST_REF",                      /* 0x0bc2 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_DITHER",                              /* 0x0bd0 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_BLEND_DST",                           /* 0x0be0 */
    "GL_BLEND_SRC",                           /* 0x0be1 */
    "GL_BLEND",                               /* 0x0be2 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_LOGIC_OP_MODE",                       /* 0x0bf0 */
    "GL_LOGIC_OP",                            /* 0x0bf1 */
    "GL_COLOR_LOGIC_OP",                      /* 0x0bf2 */
    "GL_AUX_BUFFERS",                         /* 0x0c00 */
    "GL_DRAW_BUFFER",                         /* 0x0c01 */
    "GL_READ_BUFFER",                         /* 0x0c02 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_SCISSOR_BOX",                         /* 0x0c10 */
    "GL_SCISSOR_TEST",                        /* 0x0c11 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_INDEX_CLEAR_VALUE",                   /* 0x0c20 */
    "GL_INDEX_WRITEMASK",                     /* 0x0c21 */
    "GL_COLOR_CLEAR_VALUE",                   /* 0x0c22 */
    "GL_COLOR_WRITEMASK",                     /* 0x0c23 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_INDEX_MODE",                          /* 0x0c30 */
    "GL_RGBA_MODE",                           /* 0x0c31 */
    "GL_DOUBLEBUFFER",                        /* 0x0c32 */
    "GL_STEREO",                              /* 0x0c33 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_RENDER_MODE",                         /* 0x0c40 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_PERSPECTIVE_CORRECTION_HINT",         /* 0x0c50 */
    "GL_POINT_SMOOTH_HINT",                   /* 0x0c51 */
    "GL_LINE_SMOOTH_HINT",                    /* 0x0c52 */
    "GL_POLYGON_SMOOTH_HINT",                 /* 0x0c53 */
    "GL_FOG_HINT",                            /* 0x0c54 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_TEXTURE_GEN_S",                       /* 0x0c60 */
    "GL_TEXTURE_GEN_T",                       /* 0x0c61 */
    "GL_TEXTURE_GEN_R",                       /* 0x0c62 */
    "GL_TEXTURE_GEN_Q",                       /* 0x0c63 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_PIXEL_MAP_I_TO_I",                    /* 0x0c70 */
    "GL_PIXEL_MAP_S_TO_S",                    /* 0x0c71 */
    "GL_PIXEL_MAP_I_TO_R",                    /* 0x0c72 */
    "GL_PIXEL_MAP_I_TO_G",                    /* 0x0c73 */
    "GL_PIXEL_MAP_I_TO_B",                    /* 0x0c74 */
    "GL_PIXEL_MAP_I_TO_A",                    /* 0x0c75 */
    "GL_PIXEL_MAP_R_TO_R",                    /* 0x0c76 */
    "GL_PIXEL_MAP_G_TO_G",                    /* 0x0c77 */
    "GL_PIXEL_MAP_B_TO_B",                    /* 0x0c78 */
    "GL_PIXEL_MAP_A_TO_A",                    /* 0x0c79 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_PIXEL_MAP_I_TO_I_SIZE",               /* 0x0cb0 */
    "GL_PIXEL_MAP_S_TO_S_SIZE",               /* 0x0cb1 */
    "GL_PIXEL_MAP_I_TO_R_SIZE",               /* 0x0cb2 */
    "GL_PIXEL_MAP_I_TO_G_SIZE",               /* 0x0cb3 */
    "GL_PIXEL_MAP_I_TO_B_SIZE",               /* 0x0cb4 */
    "GL_PIXEL_MAP_I_TO_A_SIZE",               /* 0x0cb5 */
    "GL_PIXEL_MAP_R_TO_R_SIZE",               /* 0x0cb6 */
    "GL_PIXEL_MAP_G_TO_G_SIZE",               /* 0x0cb7 */
    "GL_PIXEL_MAP_B_TO_B_SIZE",               /* 0x0cb8 */
    "GL_PIXEL_MAP_A_TO_A_SIZE",               /* 0x0cb9 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_UNPACK_SWAP_BYTES",                   /* 0x0cf0 */
    "GL_UNPACK_LSB_FIRST",                    /* 0x0cf1 */
    "GL_UNPACK_ROW_LENGTH",                   /* 0x0cf2 */
    "GL_UNPACK_SKIP_ROWS",                    /* 0x0cf3 */
    "GL_UNPACK_SKIP_PIXELS",                  /* 0x0cf4 */
    "GL_UNPACK_ALIGNMENT",                    /* 0x0cf5 */
    "GL_PACK_SWAP_BYTES",                     /* 0x0d00 */
    "GL_PACK_LSB_FIRST",                      /* 0x0d01 */
    "GL_PACK_ROW_LENGTH",                     /* 0x0d02 */
    "GL_PACK_SKIP_ROWS",                      /* 0x0d03 */
    "GL_PACK_SKIP_PIXELS",                    /* 0x0d04 */
    "GL_PACK_ALIGNMENT",                      /* 0x0d05 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_MAP_COLOR",                           /* 0x0d10 */
    "GL_MAP_STENCIL",                         /* 0x0d11 */
    "GL_INDEX_SHIFT",                         /* 0x0d12 */
    "GL_INDEX_OFFSET",                        /* 0x0d13 */
    "GL_RED_SCALE",                           /* 0x0d14 */
    "GL_RED_BIAS",                            /* 0x0d15 */
    "GL_ZOOM_X",                              /* 0x0d16 */
    "GL_ZOOM_Y",                              /* 0x0d17 */
    "GL_GREEN_SCALE",                         /* 0x0d18 */
    "GL_GREEN_BIAS",                          /* 0x0d19 */
    "GL_BLUE_SCALE",                          /* 0x0d1a */
    "GL_BLUE_BIAS",                           /* 0x0d1b */
    "GL_ALPHA_SCALE",                         /* 0x0d1c */
    "GL_ALPHA_BIAS",                          /* 0x0d1d */
    "GL_DEPTH_SCALE",                         /* 0x0d1e */
    "GL_DEPTH_BIAS",                          /* 0x0d1f */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_MAX_EVAL_ORDER",                      /* 0x0d30 */
    "GL_MAX_LIGHTS",                          /* 0x0d31 */
    "GL_MAX_CLIP_PLANES",                     /* 0x0d32 */
    "GL_MAX_TEXTURE_SIZE",                    /* 0x0d33 */
    "GL_MAX_PIXEL_MAP_TABLE",                 /* 0x0d34 */
    "GL_MAX_ATTRIB_STACK_DEPTH",              /* 0x0d35 */
    "GL_MAX_MODELVIEW_STACK_DEPTH",           /* 0x0d36 */
    "GL_MAX_NAME_STACK_DEPTH",                /* 0x0d37 */
    "GL_MAX_PROJECTION_STACK_DEPTH",          /* 0x0d38 */
    "GL_MAX_TEXTURE_STACK_DEPTH",             /* 0x0d39 */
    "GL_MAX_VIEWPORT_DIMS",                   /* 0x0d3a */
    "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH",       /* 0x0d3b */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_SUBPIXEL_BITS",                       /* 0x0d50 */
    "GL_INDEX_BITS",                          /* 0x0d51 */
    "GL_RED_BITS",                            /* 0x0d52 */
    "GL_GREEN_BITS",                          /* 0x0d53 */
    "GL_BLUE_BITS",                           /* 0x0d54 */
    "GL_ALPHA_BITS",                          /* 0x0d55 */
    "GL_DEPTH_BITS",                          /* 0x0d56 */
    "GL_STENCIL_BITS",                        /* 0x0d57 */
    "GL_ACCUM_RED_BITS",                      /* 0x0d58 */
    "GL_ACCUM_GREEN_BITS",                    /* 0x0d59 */
    "GL_ACCUM_BLUE_BITS",                     /* 0x0d5a */
    "GL_ACCUM_ALPHA_BITS",                    /* 0x0d5b */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_NAME_STACK_DEPTH",                    /* 0x0d70 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_AUTO_NORMAL",                         /* 0x0d80 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_MAP1_COLOR_4",                        /* 0x0d90 */
    "GL_MAP1_INDEX",                          /* 0x0d91 */
    "GL_MAP1_NORMAL",                         /* 0x0d92 */
    "GL_MAP1_TEXTURE_COORD_1",                /* 0x0d93 */
    "GL_MAP1_TEXTURE_COORD_2",                /* 0x0d94 */
    "GL_MAP1_TEXTURE_COORD_3",                /* 0x0d95 */
    "GL_MAP1_TEXTURE_COORD_4",                /* 0x0d96 */
    "GL_MAP1_VERTEX_3",                       /* 0x0d97 */
    "GL_MAP1_VERTEX_4",                       /* 0x0d98 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_MAP2_COLOR_4",                        /* 0x0db0 */
    "GL_MAP2_INDEX",                          /* 0x0db1 */
    "GL_MAP2_NORMAL",                         /* 0x0db2 */
    "GL_MAP2_TEXTURE_COORD_1",                /* 0x0db3 */
    "GL_MAP2_TEXTURE_COORD_2",                /* 0x0db4 */
    "GL_MAP2_TEXTURE_COORD_3",                /* 0x0db5 */
    "GL_MAP2_TEXTURE_COORD_4",                /* 0x0db6 */
    "GL_MAP2_VERTEX_3",                       /* 0x0db7 */
    "GL_MAP2_VERTEX_4",                       /* 0x0db8 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_MAP1_GRID_DOMAIN",                    /* 0x0dd0 */
    "GL_MAP1_GRID_SEGMENTS",                  /* 0x0dd1 */
    "GL_MAP2_GRID_DOMAIN",                    /* 0x0dd2 */
    "GL_MAP2_GRID_SEGMENTS",                  /* 0x0dd3 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_TEXTURE_1D",                          /* 0x0de0 */
    "GL_TEXTURE_2D",                          /* 0x0de1 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_FEEDBACK_BUFFER_POINTER",             /* 0x0df0 */
    "GL_FEEDBACK_BUFFER_SIZE",                /* 0x0df1 */
    "GL_FEEDBACK_BUFFER_TYPE",                /* 0x0df2 */
    "GL_SELECTION_BUFFER_POINTER",            /* 0x0df3 */
    "GL_SELECTION_BUFFER_SIZE",               /* 0x0df4 */
    "GL_TEXTURE_WIDTH",                       /* 0x1000 */
    "GL_TEXTURE_HEIGHT",                      /* 0x1001 */
     NULL,
    "GL_TEXTURE_COMPONENTS",                  /* 0x1003 */
    "GL_TEXTURE_BORDER_COLOR",                /* 0x1004 */
    "GL_TEXTURE_BORDER",                      /* 0x1005 */
    "GL_DONT_CARE",                           /* 0x1100 */
    "GL_FASTEST",                             /* 0x1101 */
    "GL_NICEST",                              /* 0x1102 */
    "GL_AMBIENT",                             /* 0x1200 */
    "GL_DIFFUSE",                             /* 0x1201 */
    "GL_SPECULAR",                            /* 0x1202 */
    "GL_POSITION",                            /* 0x1203 */
    "GL_SPOT_DIRECTION",                      /* 0x1204 */
    "GL_SPOT_EXPONENT",                       /* 0x1205 */
    "GL_SPOT_CUTOFF",                         /* 0x1206 */
    "GL_CONSTANT_ATTENUATION",                /* 0x1207 */
    "GL_LINEAR_ATTENUATION",                  /* 0x1208 */
    "GL_QUADRATIC_ATTENUATION",               /* 0x1209 */
    "GL_COMPILE",                             /* 0x1300 */
    "GL_COMPILE_AND_EXECUTE",                 /* 0x1301 */
    "GL_BYTE",                                /* 0x1400 */
    "GL_UNSIGNED_BYTE",                       /* 0x1401 */
    "GL_SHORT",                               /* 0x1402 */
    "GL_UNSIGNED_SHORT",                      /* 0x1403 */
    "GL_INT",                                 /* 0x1404 */
    "GL_UNSIGNED_INT",                        /* 0x1405 */
    "GL_FLOAT",                               /* 0x1406 */
    "GL_2_BYTES",                             /* 0x1407 */
    "GL_3_BYTES",                             /* 0x1408 */
    "GL_4_BYTES",                             /* 0x1409 */
    "GL_DOUBLE_EXT",                          /* 0x140a */
    "GL_CLEAR",                               /* 0x1500 */
    "GL_AND",                                 /* 0x1501 */
    "GL_AND_REVERSE",                         /* 0x1502 */
    "GL_COPY",                                /* 0x1503 */
    "GL_AND_INVERTED",                        /* 0x1504 */
    "GL_NOOP",                                /* 0x1505 */
    "GL_XOR",                                 /* 0x1506 */
    "GL_OR",                                  /* 0x1507 */
    "GL_NOR",                                 /* 0x1508 */
    "GL_EQUIV",                               /* 0x1509 */
    "GL_INVERT",                              /* 0x150a */
    "GL_OR_REVERSE",                          /* 0x150b */
    "GL_COPY_INVERTED",                       /* 0x150c */
    "GL_OR_INVERTED",                         /* 0x150d */
    "GL_NAND",                                /* 0x150e */
    "GL_SET",                                 /* 0x150f */
    "GL_EMISSION",                            /* 0x1600 */
    "GL_SHININESS",                           /* 0x1601 */
    "GL_AMBIENT_AND_DIFFUSE",                 /* 0x1602 */
    "GL_COLOR_INDEXES",                       /* 0x1603 */
    "GL_MODELVIEW",                           /* 0x1700 */
    "GL_PROJECTION",                          /* 0x1701 */
    "GL_TEXTURE",                             /* 0x1702 */
    "GL_COLOR",                               /* 0x1800 */
    "GL_DEPTH",                               /* 0x1801 */
    "GL_STENCIL",                             /* 0x1802 */
    "GL_COLOR_INDEX",                         /* 0x1900 */
    "GL_STENCIL_INDEX",                       /* 0x1901 */
    "GL_DEPTH_COMPONENT",                     /* 0x1902 */
    "GL_RED",                                 /* 0x1903 */
    "GL_GREEN",                               /* 0x1904 */
    "GL_BLUE",                                /* 0x1905 */
    "GL_ALPHA",                               /* 0x1906 */
    "GL_RGB",                                 /* 0x1907 */
    "GL_RGBA",                                /* 0x1908 */
    "GL_LUMINANCE",                           /* 0x1909 */
    "GL_LUMINANCE_ALPHA",                     /* 0x190a */
    "GL_BITMAP",                              /* 0x1a00 */
    "GL_POINT",                               /* 0x1b00 */
    "GL_LINE",                                /* 0x1b01 */
    "GL_FILL",                                /* 0x1b02 */
    "GL_RENDER",                              /* 0x1c00 */
    "GL_FEEDBACK",                            /* 0x1c01 */
    "GL_SELECT",                              /* 0x1c02 */
    "GL_FLAT",                                /* 0x1d00 */
    "GL_SMOOTH",                              /* 0x1d01 */
    "GL_KEEP",                                /* 0x1e00 */
    "GL_REPLACE",                             /* 0x1e01 */
    "GL_INCR",                                /* 0x1e02 */
    "GL_DECR",                                /* 0x1e03 */
    "GL_VENDOR",                              /* 0x1f00 */
    "GL_RENDERER",                            /* 0x1f01 */
    "GL_VERSION",                             /* 0x1f02 */
    "GL_EXTENSIONS",                          /* 0x1f03 */
    "GL_S",                                   /* 0x2000 */
    "GL_T",                                   /* 0x2001 */
    "GL_R",                                   /* 0x2002 */
    "GL_Q",                                   /* 0x2003 */
    "GL_MODULATE",                            /* 0x2100 */
    "GL_DECAL",                               /* 0x2101 */
    "GL_TEXTURE_ENV_MODE",                    /* 0x2200 */
    "GL_TEXTURE_ENV_COLOR",                   /* 0x2201 */
    "GL_TEXTURE_ENV",                         /* 0x2300 */
    "GL_EYE_LINEAR",                          /* 0x2400 */
    "GL_OBJECT_LINEAR",                       /* 0x2401 */
    "GL_SPHERE_MAP",                          /* 0x2402 */
    "GL_TEXTURE_GEN_MODE",                    /* 0x2500 */
    "GL_OBJECT_PLANE",                        /* 0x2501 */
    "GL_EYE_PLANE",                           /* 0x2502 */
    "GL_NEAREST",                             /* 0x2600 */
    "GL_LINEAR",                              /* 0x2601 */
    "GL_NEAREST_MIPMAP_NEAREST",              /* 0x2700 */
    "GL_LINEAR_MIPMAP_NEAREST",               /* 0x2701 */
    "GL_NEAREST_MIPMAP_LINEAR",               /* 0x2702 */
    "GL_LINEAR_MIPMAP_LINEAR",                /* 0x2703 */
    "GL_TEXTURE_MAG_FILTER",                  /* 0x2800 */
    "GL_TEXTURE_MIN_FILTER",                  /* 0x2801 */
    "GL_TEXTURE_WRAP_S",                      /* 0x2802 */
    "GL_TEXTURE_WRAP_T",                      /* 0x2803 */
    "GL_CLAMP",                               /* 0x2900 */
    "GL_REPEAT",                              /* 0x2901 */
    "GL_POLYGON_OFFSET_UNITS",                /* 0x2a00 */
    "GL_POLYGON_OFFSET_POINT",                /* 0x2a01 */
    "GL_POLYGON_OFFSET_LINE",                 /* 0x2a02 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_R3_G3_B2",                            /* 0x2a10 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_V2F",                                 /* 0x2a20 */
    "GL_V3F",                                 /* 0x2a21 */
    "GL_C4UB_V2F",                            /* 0x2a22 */
    "GL_C4UB_V3F",                            /* 0x2a23 */
    "GL_C3F_V3F",                             /* 0x2a24 */
    "GL_N3F_V3F",                             /* 0x2a25 */
    "GL_C4F_N3F_V3F",                         /* 0x2a26 */
    "GL_T2F_V3F",                             /* 0x2a27 */
    "GL_T4F_V4F",                             /* 0x2a28 */
    "GL_T2F_C4UB_V3F",                        /* 0x2a29 */
    "GL_T2F_C3F_V3F",                         /* 0x2a2a */
    "GL_T2F_N3F_V3F",                         /* 0x2a2b */
    "GL_T2F_C4F_N3F_V3F",                     /* 0x2a2c */
    "GL_T4F_C4F_N3F_V4F",                     /* 0x2a2d */
    "GL_CLIP_PLANE0",                         /* 0x3000 */
    "GL_CLIP_PLANE1",                         /* 0x3001 */
    "GL_CLIP_PLANE2",                         /* 0x3002 */
    "GL_CLIP_PLANE3",                         /* 0x3003 */
    "GL_CLIP_PLANE4",                         /* 0x3004 */
    "GL_CLIP_PLANE5",                         /* 0x3005 */
    "GL_LIGHT0",                              /* 0x4000 */
    "GL_LIGHT1",                              /* 0x4001 */
    "GL_LIGHT2",                              /* 0x4002 */
    "GL_LIGHT3",                              /* 0x4003 */
    "GL_LIGHT4",                              /* 0x4004 */
    "GL_LIGHT5",                              /* 0x4005 */
    "GL_LIGHT6",                              /* 0x4006 */
    "GL_LIGHT7",                              /* 0x4007 */
    "GL_ABGR_EXT",                            /* 0x8000 */
    "GL_CONSTANT_COLOR_EXT",                  /* 0x8001 */
    "GL_ONE_MINUS_CONSTANT_COLOR_EXT",        /* 0x8002 */
    "GL_CONSTANT_ALPHA_EXT",                  /* 0x8003 */
    "GL_ONE_MINUS_CONSTANT_ALPHA_EXT",        /* 0x8004 */
    "GL_BLEND_COLOR_EXT",                     /* 0x8005 */
    "GL_FUNC_ADD_EXT",                        /* 0x8006 */
    "GL_MIN_EXT",                             /* 0x8007 */
    "GL_MAX_EXT",                             /* 0x8008 */
    "GL_BLEND_EQUATION_EXT",                  /* 0x8009 */
    "GL_FUNC_SUBTRACT_EXT",                   /* 0x800a */
    "GL_FUNC_REVERSE_SUBTRACT_EXT",           /* 0x800b */
    "GL_CMYK_EXT",                            /* 0x800c */
    "GL_CMYKA_EXT",                           /* 0x800d */
    "GL_PACK_CMYK_HINT_EXT",                  /* 0x800e */
    "GL_UNPACK_CMYK_HINT_EXT",                /* 0x800f */
    "GL_CONVOLUTION_1D_EXT",                  /* 0x8010 */
    "GL_CONVOLUTION_2D_EXT",                  /* 0x8011 */
    "GL_SEPARABLE_2D_EXT",                    /* 0x8012 */
    "GL_CONVOLUTION_BORDER_MODE_EXT",         /* 0x8013 */
    "GL_CONVOLUTION_FILTER_SCALE_EXT",        /* 0x8014 */
    "GL_CONVOLUTION_FILTER_BIAS_EXT",         /* 0x8015 */
    "GL_REDUCE_EXT",                          /* 0x8016 */
    "GL_CONVOLUTION_FORMAT_EXT",              /* 0x8017 */
    "GL_CONVOLUTION_WIDTH_EXT",               /* 0x8018 */
    "GL_CONVOLUTION_HEIGHT_EXT",              /* 0x8019 */
    "GL_MAX_CONVOLUTION_WIDTH_EXT",           /* 0x801a */
    "GL_MAX_CONVOLUTION_HEIGHT_EXT",          /* 0x801b */
    "GL_POST_CONVOLUTION_RED_SCALE_EXT",      /* 0x801c */
    "GL_POST_CONVOLUTION_GREEN_SCALE_EXT",    /* 0x801d */
    "GL_POST_CONVOLUTION_BLUE_SCALE_EXT",     /* 0x801e */
    "GL_POST_CONVOLUTION_ALPHA_SCALE_EXT",    /* 0x801f */
    "GL_POST_CONVOLUTION_RED_BIAS_EXT",       /* 0x8020 */
    "GL_POST_CONVOLUTION_GREEN_BIAS_EXT",     /* 0x8021 */
    "GL_POST_CONVOLUTION_BLUE_BIAS_EXT",      /* 0x8022 */
    "GL_POST_CONVOLUTION_ALPHA_BIAS_EXT",     /* 0x8023 */
    "GL_HISTOGRAM_EXT",                       /* 0x8024 */
    "GL_PROXY_HISTOGRAM_EXT",                 /* 0x8025 */
    "GL_HISTOGRAM_WIDTH_EXT",                 /* 0x8026 */
    "GL_HISTOGRAM_FORMAT_EXT",                /* 0x8027 */
    "GL_HISTOGRAM_RED_SIZE_EXT",              /* 0x8028 */
    "GL_HISTOGRAM_GREEN_SIZE_EXT",            /* 0x8029 */
    "GL_HISTOGRAM_BLUE_SIZE_EXT",             /* 0x802a */
    "GL_HISTOGRAM_ALPHA_SIZE_EXT",            /* 0x802b */
    "GL_HISTOGRAM_LUMINANCE_SIZE_EXT",        /* 0x802c */
    "GL_HISTOGRAM_SINK_EXT",                  /* 0x802d */
    "GL_MINMAX_EXT",                          /* 0x802e */
    "GL_MINMAX_FORMAT_EXT",                   /* 0x802f */
    "GL_MINMAX_SINK_EXT",                     /* 0x8030 */
    "GL_TABLE_TOO_LARGE_EXT",                 /* 0x8031 */
    "GL_UNSIGNED_BYTE_3_3_2_EXT",             /* 0x8032 */
    "GL_UNSIGNED_SHORT_4_4_4_4_EXT",          /* 0x8033 */
    "GL_UNSIGNED_SHORT_5_5_5_1_EXT",          /* 0x8034 */
    "GL_UNSIGNED_INT_8_8_8_8_EXT",            /* 0x8035 */
    "GL_UNSIGNED_INT_10_10_10_2_EXT",         /* 0x8036 */
    "GL_POLYGON_OFFSET_EXT",                  /* 0x8037 */
    "GL_POLYGON_OFFSET_FACTOR_EXT",           /* 0x8038 */
    "GL_POLYGON_OFFSET_BIAS_EXT",             /* 0x8039 */
    "GL_RESCALE_NORMAL_EXT",                  /* 0x803a */
    "GL_ALPHA4_EXT",                          /* 0x803b */
    "GL_ALPHA8_EXT",                          /* 0x803c */
    "GL_ALPHA12_EXT",                         /* 0x803d */
    "GL_ALPHA16_EXT",                         /* 0x803e */
    "GL_LUMINANCE4_EXT",                      /* 0x803f */
    "GL_LUMINANCE8_EXT",                      /* 0x8040 */
    "GL_LUMINANCE12_EXT",                     /* 0x8041 */
    "GL_LUMINANCE16_EXT",                     /* 0x8042 */
    "GL_LUMINANCE4_ALPHA4_EXT",               /* 0x8043 */
    "GL_LUMINANCE6_ALPHA2_EXT",               /* 0x8044 */
    "GL_LUMINANCE8_ALPHA8_EXT",               /* 0x8045 */
    "GL_LUMINANCE12_ALPHA4_EXT",              /* 0x8046 */
    "GL_LUMINANCE12_ALPHA12_EXT",             /* 0x8047 */
    "GL_LUMINANCE16_ALPHA16_EXT",             /* 0x8048 */
    "GL_INTENSITY_EXT",                       /* 0x8049 */
    "GL_INTENSITY4_EXT",                      /* 0x804a */
    "GL_INTENSITY8_EXT",                      /* 0x804b */
    "GL_INTENSITY12_EXT",                     /* 0x804c */
    "GL_INTENSITY16_EXT",                     /* 0x804d */
    "GL_RGB2_EXT",                            /* 0x804e */
    "GL_RGB4_EXT",                            /* 0x804f */
    "GL_RGB5_EXT",                            /* 0x8050 */
    "GL_RGB8_EXT",                            /* 0x8051 */
    "GL_RGB10_EXT",                           /* 0x8052 */
    "GL_RGB12_EXT",                           /* 0x8053 */
    "GL_RGB16_EXT",                           /* 0x8054 */
    "GL_RGBA2_EXT",                           /* 0x8055 */
    "GL_RGBA4_EXT",                           /* 0x8056 */
    "GL_RGB5_A1_EXT",                         /* 0x8057 */
    "GL_RGBA8_EXT",                           /* 0x8058 */
    "GL_RGB10_A2_EXT",                        /* 0x8059 */
    "GL_RGBA12_EXT",                          /* 0x805a */
    "GL_RGBA16_EXT",                          /* 0x805b */
    "GL_TEXTURE_RED_SIZE_EXT",                /* 0x805c */
    "GL_TEXTURE_GREEN_SIZE_EXT",              /* 0x805d */
    "GL_TEXTURE_BLUE_SIZE_EXT",               /* 0x805e */
    "GL_TEXTURE_ALPHA_SIZE_EXT",              /* 0x805f */
    "GL_TEXTURE_LUMINANCE_SIZE_EXT",          /* 0x8060 */
    "GL_TEXTURE_INTENSITY_SIZE_EXT",          /* 0x8061 */
    "GL_REPLACE_EXT",                         /* 0x8062 */
    "GL_PROXY_TEXTURE_1D_EXT",                /* 0x8063 */
    "GL_PROXY_TEXTURE_2D_EXT",                /* 0x8064 */
    "GL_TEXTURE_TOO_LARGE_EXT",               /* 0x8065 */
    "GL_TEXTURE_PRIORITY_EXT",                /* 0x8066 */
    "GL_TEXTURE_RESIDENT_EXT",                /* 0x8067 */
    "GL_TEXTURE_1D_BINDING_EXT",              /* 0x8068 */
    "GL_TEXTURE_2D_BINDING_EXT",              /* 0x8069 */
    "GL_TEXTURE_3D_BINDING_EXT",              /* 0x806a */
    "GL_PACK_SKIP_IMAGES_EXT",                /* 0x806b */
    "GL_PACK_IMAGE_HEIGHT_EXT",               /* 0x806c */
    "GL_UNPACK_SKIP_IMAGES_EXT",              /* 0x806d */
    "GL_UNPACK_IMAGE_HEIGHT_EXT",             /* 0x806e */
    "GL_TEXTURE_3D_EXT",                      /* 0x806f */
    "GL_PROXY_TEXTURE_3D_EXT",                /* 0x8070 */
    "GL_TEXTURE_DEPTH_EXT",                   /* 0x8071 */
    "GL_TEXTURE_WRAP_R_EXT",                  /* 0x8072 */
    "GL_MAX_3D_TEXTURE_SIZE_EXT",             /* 0x8073 */
    "GL_VERTEX_ARRAY_EXT",                    /* 0x8074 */
    "GL_NORMAL_ARRAY_EXT",                    /* 0x8075 */
    "GL_COLOR_ARRAY_EXT",                     /* 0x8076 */
    "GL_INDEX_ARRAY_EXT",                     /* 0x8077 */
    "GL_TEXTURE_COORD_ARRAY_EXT",             /* 0x8078 */
    "GL_EDGE_FLAG_ARRAY_EXT",                 /* 0x8079 */
    "GL_VERTEX_ARRAY_SIZE_EXT",               /* 0x807a */
    "GL_VERTEX_ARRAY_TYPE_EXT",               /* 0x807b */
    "GL_VERTEX_ARRAY_STRIDE_EXT",             /* 0x807c */
    "GL_VERTEX_ARRAY_COUNT_EXT",              /* 0x807d */
    "GL_NORMAL_ARRAY_TYPE_EXT",               /* 0x807e */
    "GL_NORMAL_ARRAY_STRIDE_EXT",             /* 0x807f */
    "GL_NORMAL_ARRAY_COUNT_EXT",              /* 0x8080 */
    "GL_COLOR_ARRAY_SIZE_EXT",                /* 0x8081 */
    "GL_COLOR_ARRAY_TYPE_EXT",                /* 0x8082 */
    "GL_COLOR_ARRAY_STRIDE_EXT",              /* 0x8083 */
    "GL_COLOR_ARRAY_COUNT_EXT",               /* 0x8084 */
    "GL_INDEX_ARRAY_TYPE_EXT",                /* 0x8085 */
    "GL_INDEX_ARRAY_STRIDE_EXT",              /* 0x8086 */
    "GL_INDEX_ARRAY_COUNT_EXT",               /* 0x8087 */
    "GL_TEXTURE_COORD_ARRAY_SIZE_EXT",        /* 0x8088 */
    "GL_TEXTURE_COORD_ARRAY_TYPE_EXT",        /* 0x8089 */
    "GL_TEXTURE_COORD_ARRAY_STRIDE_EXT",      /* 0x808a */
    "GL_TEXTURE_COORD_ARRAY_COUNT_EXT",       /* 0x808b */
    "GL_EDGE_FLAG_ARRAY_STRIDE_EXT",          /* 0x808c */
    "GL_EDGE_FLAG_ARRAY_COUNT_EXT",           /* 0x808d */
    "GL_VERTEX_ARRAY_POINTER_EXT",            /* 0x808e */
    "GL_NORMAL_ARRAY_POINTER_EXT",            /* 0x808f */
    "GL_COLOR_ARRAY_POINTER_EXT",             /* 0x8090 */
    "GL_INDEX_ARRAY_POINTER_EXT",             /* 0x8091 */
    "GL_TEXTURE_COORD_ARRAY_POINTER_EXT",     /* 0x8092 */
    "GL_EDGE_FLAG_ARRAY_POINTER_EXT",         /* 0x8093 */
    "GL_INTERLACE_SGIX",                      /* 0x8094 */
    "GL_DETAIL_TEXTURE_2D_SGIS",              /* 0x8095 */
    "GL_DETAIL_TEXTURE_2D_BINDING_SGIS",      /* 0x8096 */
    "GL_LINEAR_DETAIL_SGIS",                  /* 0x8097 */
    "GL_LINEAR_DETAIL_ALPHA_SGIS",            /* 0x8098 */
    "GL_LINEAR_DETAIL_COLOR_SGIS",            /* 0x8099 */
    "GL_DETAIL_TEXTURE_LEVEL_SGIS",           /* 0x809a */
    "GL_DETAIL_TEXTURE_MODE_SGIS",            /* 0x809b */
    "GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS",     /* 0x809c */
    "GL_MULTISAMPLE_SGIS",                    /* 0x809d */
    "GL_SAMPLE_ALPHA_TO_MASK_SGIS",           /* 0x809e */
    "GL_SAMPLE_ALPHA_TO_ONE_SGIS",            /* 0x809f */
    "GL_SAMPLE_MASK_SGIS",                    /* 0x80a0 */
    "GL_1PASS_SGIS",                          /* 0x80a1 */
    "GL_2PASS_0_SGIS",                        /* 0x80a2 */
    "GL_2PASS_1_SGIS",                        /* 0x80a3 */
    "GL_4PASS_0_SGIS",                        /* 0x80a4 */
    "GL_4PASS_1_SGIS",                        /* 0x80a5 */
    "GL_4PASS_2_SGIS",                        /* 0x80a6 */
    "GL_4PASS_3_SGIS",                        /* 0x80a7 */
    "GL_SAMPLE_BUFFERS_SGIS",                 /* 0x80a8 */
    "GL_SAMPLES_SGIS",                        /* 0x80a9 */
    "GL_SAMPLE_MASK_VALUE_SGIS",              /* 0x80aa */
    "GL_SAMPLE_MASK_INVERT_SGIS",             /* 0x80ab */
    "GL_SAMPLE_PATTERN_SGIS",                 /* 0x80ac */
    "GL_LINEAR_SHARPEN_SGIS",                 /* 0x80ad */
    "GL_LINEAR_SHARPEN_ALPHA_SGIS",           /* 0x80ae */
    "GL_LINEAR_SHARPEN_COLOR_SGIS",           /* 0x80af */
    "GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS",    /* 0x80b0 */
    "GL_COLOR_MATRIX_SGI",                    /* 0x80b1 */
    "GL_COLOR_MATRIX_STACK_DEPTH_SGI",        /* 0x80b2 */
    "GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI",    /* 0x80b3 */
    "GL_POST_COLOR_MATRIX_RED_SCALE_SGI",     /* 0x80b4 */
    "GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI",   /* 0x80b5 */
    "GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI",    /* 0x80b6 */
    "GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI",   /* 0x80b7 */
    "GL_POST_COLOR_MATRIX_RED_BIAS_SGI",      /* 0x80b8 */
    "GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI",    /* 0x80b9 */
    "GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI",     /* 0x80ba */
    "GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI",    /* 0x80bb */
    "GL_TEXTURE_COLOR_TABLE_SGI",             /* 0x80bc */
    "GL_PROXY_TEXTURE_COLOR_TABLE_SGI",       /* 0x80bd */
    "GL_TEXTURE_ENV_BIAS_SGIX",               /* 0x80be */
    "GL_SHADOW_AMBIENT_SGIX",                 /* 0x80bf */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_COLOR_TABLE_SGI",                     /* 0x80d0 */
    "GL_POST_CONVOLUTION_COLOR_TABLE_SGI",    /* 0x80d1 */
    "GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI",   /* 0x80d2 */
    "GL_PROXY_COLOR_TABLE_SGI",               /* 0x80d3 */
    "GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI",  /* 0x80d4 */
    "GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI",  /* 0x80d5 */
    "GL_COLOR_TABLE_SCALE_SGI",               /* 0x80d6 */
    "GL_COLOR_TABLE_BIAS_SGI",                /* 0x80d7 */
    "GL_COLOR_TABLE_FORMAT_SGI",              /* 0x80d8 */
    "GL_COLOR_TABLE_WIDTH_SGI",               /* 0x80d9 */
    "GL_COLOR_TABLE_RED_SIZE_SGI",            /* 0x80da */
    "GL_COLOR_TABLE_GREEN_SIZE_SGI",          /* 0x80db */
    "GL_COLOR_TABLE_BLUE_SIZE_SGI",           /* 0x80dc */
    "GL_COLOR_TABLE_ALPHA_SIZE_SGI",          /* 0x80dd */
    "GL_COLOR_TABLE_LUMINANCE_SIZE_SGI",      /* 0x80de */
    "GL_COLOR_TABLE_INTENSITY_SIZE_SGI",      /* 0x80df */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_DUAL_ALPHA4_SGIS",                    /* 0x8110 */
    "GL_DUAL_ALPHA8_SGIS",                    /* 0x8111 */
    "GL_DUAL_ALPHA12_SGIS",                   /* 0x8112 */
    "GL_DUAL_ALPHA16_SGIS",                   /* 0x8113 */
    "GL_DUAL_LUMINANCE4_SGIS",                /* 0x8114 */
    "GL_DUAL_LUMINANCE8_SGIS",                /* 0x8115 */
    "GL_DUAL_LUMINANCE12_SGIS",               /* 0x8116 */
    "GL_DUAL_LUMINANCE16_SGIS",               /* 0x8117 */
    "GL_DUAL_INTENSITY4_SGIS",                /* 0x8118 */
    "GL_DUAL_INTENSITY8_SGIS",                /* 0x8119 */
    "GL_DUAL_INTENSITY12_SGIS",               /* 0x811a */
    "GL_DUAL_INTENSITY16_SGIS",               /* 0x811b */
    "GL_DUAL_LUMINANCE_ALPHA4_SGIS",          /* 0x811c */
    "GL_DUAL_LUMINANCE_ALPHA8_SGIS",          /* 0x811d */
    "GL_QUAD_ALPHA4_SGIS",                    /* 0x811e */
    "GL_QUAD_ALPHA8_SGIS",                    /* 0x811f */
    "GL_QUAD_LUMINANCE4_SGIS",                /* 0x8120 */
    "GL_QUAD_LUMINANCE8_SGIS",                /* 0x8121 */
    "GL_QUAD_INTENSITY4_SGIS",                /* 0x8122 */
    "GL_QUAD_INTENSITY8_SGIS",                /* 0x8123 */
    "GL_DUAL_TEXTURE_SELECT_SGIS",            /* 0x8124 */
    "GL_QUAD_TEXTURE_SELECT_SGIS",            /* 0x8125 */
    "GL_POINT_SIZE_MIN_SGIS",                 /* 0x8126 */
    "GL_POINT_SIZE_MAX_SGIS",                 /* 0x8127 */
    "GL_POINT_FADE_THRESHOLD_SIZE_SGIS",      /* 0x8128 */
    "GL_DISTANCE_ATTENUATION_SGIS",           /* 0x8129 */
    "GL_FOG_FUNC_SGIS",                       /* 0x812a */
    "GL_FOG_FUNC_POINTS_SGIS",                /* 0x812b */
    "GL_MAX_FOG_FUNC_POINTS_SGIS",            /* 0x812c */
    "GL_CLAMP_TO_BORDER_SGIS",                /* 0x812d */
    "GL_TEXTURE_MULTI_BUFFER_HINT_SGIX",      /* 0x812e */
    "GL_CLAMP_TO_EDGE_SGIS",                  /* 0x812f */
    "GL_PACK_SKIP_VOLUMES_SGIS",              /* 0x8130 */
    "GL_PACK_IMAGE_DEPTH_SGIS",               /* 0x8131 */
    "GL_UNPACK_SKIP_VOLUMES_SGIS",            /* 0x8132 */
    "GL_UNPACK_IMAGE_DEPTH_SGIS",             /* 0x8133 */
    "GL_TEXTURE_4D_SGIS",                     /* 0x8134 */
    "GL_PROXY_TEXTURE_4D_SGIS",               /* 0x8135 */
    "GL_TEXTURE_4DSIZE_SGIS",                 /* 0x8136 */
    "GL_TEXTURE_WRAP_Q_SGIS",                 /* 0x8137 */
    "GL_MAX_4D_TEXTURE_SIZE_SGIS",            /* 0x8138 */
    "GL_PIXEL_TEX_GEN_SGIX",                  /* 0x8139 */
    "GL_TEXTURE_MIN_LOD_SGIS",                /* 0x813a */
    "GL_TEXTURE_MAX_LOD_SGIS",                /* 0x813b */
    "GL_TEXTURE_BASE_LEVEL_SGIS",             /* 0x813c */
    "GL_TEXTURE_MAX_LEVEL_SGIS",              /* 0x813d */
    "GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX",      /* 0x813e */
    "GL_PIXEL_TILE_CACHE_INCREMENT_SGIX",     /* 0x813f */
    "GL_PIXEL_TILE_WIDTH_SGIX",               /* 0x8140 */
    "GL_PIXEL_TILE_HEIGHT_SGIX",              /* 0x8141 */
    "GL_PIXEL_TILE_GRID_WIDTH_SGIX",          /* 0x8142 */
    "GL_PIXEL_TILE_GRID_HEIGHT_SGIX",         /* 0x8143 */
    "GL_PIXEL_TILE_GRID_DEPTH_SGIX",          /* 0x8144 */
    "GL_PIXEL_TILE_CACHE_SIZE_SGIX",          /* 0x8145 */
    "GL_FILTER4_SGIS",                        /* 0x8146 */
    "GL_TEXTURE_FILTER4_SIZE_SGIS",           /* 0x8147 */
    "GL_SPRITE_SGIX",                         /* 0x8148 */
    "GL_SPRITE_MODE_SGIX",                    /* 0x8149 */
    "GL_SPRITE_AXIS_SGIX",                    /* 0x814a */
    "GL_SPRITE_TRANSLATION_SGIX",             /* 0x814b */
    "GL_SPRITE_AXIAL_SGIX",                   /* 0x814c */
    "GL_SPRITE_OBJECT_ALIGNED_SGIX",          /* 0x814d */
    "GL_SPRITE_EYE_ALIGNED_SGIX",             /* 0x814e */
    "GL_TEXTURE_4D_BINDING_SGIS",             /* 0x814f */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_LINEAR_CLIPMAP_LINEAR_SGIX",          /* 0x8170 */
    "GL_TEXTURE_CLIPMAP_CENTER_SGIX",         /* 0x8171 */
    "GL_TEXTURE_CLIPMAP_FRAME_SGIX",          /* 0x8172 */
    "GL_TEXTURE_CLIPMAP_OFFSET_SGIX",         /* 0x8173 */
    "GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX",  /* 0x8174 */
    "GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX",     /* 0x8175 */
    "GL_TEXTURE_CLIPMAP_DEPTH_SGIX",          /* 0x8176 */
    "GL_MAX_CLIPMAP_DEPTH_SGIX",              /* 0x8177 */
    "GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX",      /* 0x8178 */
    "GL_POST_TEXTURE_FILTER_BIAS_SGIX",       /* 0x8179 */
    "GL_POST_TEXTURE_FILTER_SCALE_SGIX",      /* 0x817a */
    "GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX",  /* 0x817b */
    "GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX",  /* 0x817c */
    "GL_REFERENCE_PLANE_SGIX",                /* 0x817d */
    "GL_REFERENCE_PLANE_EQUATION_SGIX",       /* 0x817e */
    "GL_IR_INSTRUMENT1_SGIX",                 /* 0x817f */
    "GL_INSTRUMENT_BUFFER_POINTER_SGIX",      /* 0x8180 */
    "GL_INSTRUMENT_MEASUREMENTS_SGIX",        /* 0x8181 */
    "GL_LIST_PRIORITY_SGIX",                  /* 0x8182 */
    "GL_CALLIGRAPHIC_FRAGMENT_SGIX",          /* 0x8183 */
    "GL_PIXEL_TEX_GEN_Q_CEILING_SGIX",        /* 0x8184 */
    "GL_PIXEL_TEX_GEN_Q_ROUND_SGIX",          /* 0x8185 */
    "GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX",          /* 0x8186 */
    "GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX",    /* 0x8187 */
    "GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX",  /* 0x8188 */
    "GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX",         /* 0x8189 */
    "GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX",         /* 0x818a */
    "GL_FRAMEZOOM_SGIX",                      /* 0x818b */
    "GL_FRAMEZOOM_FACTOR_SGIX",               /* 0x818c */
    "GL_MAX_FRAMEZOOM_FACTOR_SGIX",           /* 0x818d */
    "GL_TEXTURE_LOD_BIAS_S_SGIX",             /* 0x818e */
    "GL_TEXTURE_LOD_BIAS_T_SGIX",             /* 0x818f */
    "GL_TEXTURE_LOD_BIAS_R_SGIX",             /* 0x8190 */
    "GL_GENERATE_MIPMAP_SGIS",                /* 0x8191 */
    "GL_GENERATE_MIPMAP_HINT_SGIS",           /* 0x8192 */
     NULL,
    "GL_GEOMETRY_DEFORMATION_SGIX",           /* 0x8194 */
    "GL_TEXTURE_DEFORMATION_SGIX",            /* 0x8195 */
    "GL_DEFORMATIONS_MASK_SGIX",              /* 0x8196 */
    "GL_MAX_DEFORMATION_ORDER_SGIX",          /* 0x8197 */
    "GL_FOG_OFFSET_SGIX",                     /* 0x8198 */
    "GL_FOG_OFFSET_VALUE_SGIX",               /* 0x8199 */
    "GL_TEXTURE_COMPARE_SGIX",                /* 0x819a */
    "GL_TEXTURE_COMPARE_OPERATOR_SGIX",       /* 0x819b */
    "GL_TEXTURE_LEQUAL_R_SGIX",               /* 0x819c */
    "GL_TEXTURE_GEQUAL_R_SGIX",               /* 0x819d */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_DEPTH_COMPONENT16_SGIX",              /* 0x81a5 */
    "GL_DEPTH_COMPONENT24_SGIX",              /* 0x81a6 */
    "GL_DEPTH_COMPONENT32_SGIX",              /* 0x81a7 */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_YCRCB_422_SGIX",                      /* 0x81bb */
    "GL_YCRCB_444_SGIX",                      /* 0x81bc */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
    "GL_EYE_DISTANCE_TO_POINT_SGIS",          /* 0x81f0 */
    "GL_OBJECT_DISTANCE_TO_POINT_SGIS",       /* 0x81f1 */
    "GL_EYE_DISTANCE_TO_LINE_SGIS",           /* 0x81f2 */
    "GL_OBJECT_DISTANCE_TO_LINE_SGIS",        /* 0x81f3 */
    "GL_EYE_POINT_SGIS",                      /* 0x81f4 */
    "GL_OBJECT_POINT_SGIS",                   /* 0x81f5 */
    "GL_EYE_LINE_SGIS",                       /* 0x81f6 */
    "GL_OBJECT_LINE_SGIS",                    /* 0x81f7 */
    "GL_POINTS",                              /* 0x0000 */
    "GL_LINES",                               /* 0x0001 */
    "GL_LINE_LOOP",                           /* 0x0002 */
    "GL_LINE_STRIP",                          /* 0x0003 */
    "GL_TRIANGLES",                           /* 0x0004 */
    "GL_TRIANGLE_STRIP",                      /* 0x0005 */
    "GL_TRIANGLE_FAN",                        /* 0x0006 */
    "GL_QUADS",                               /* 0x0007 */
    "GL_QUAD_STRIP",                          /* 0x0008 */
    "GL_POLYGON",                             /* 0x0009 */
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
    "GL_CURSOR_POSITION_CR",                  /* 0x8AF0 */
    "GL_DEFAULT_BBOX_CR",                     /* 0x8AF1 */
    "GL_SCREEN_BBOX_CR",                      /* 0x8AF2 */
    "GL_OBJECT_BBOX_CR",                      /* 0x8AF3 */
    "GL_PRINT_STRING_CR",                     /* 0x8AF4 */
    "GL_MURAL_SIZE_CR",                       /* 0x8AF5 */
    "GL_NUM_SERVERS_CR",                      /* 0x8AF6 */
    "GL_NUM_TILES_CR",                        /* 0x8AF7 */
    "GL_TILE_BOUNDS_CR",                      /* 0x8AF8 */
    "GL_VERTEX_COUNTS_CR",                    /* 0x8AF9 */
    "GL_RESET_VERTEX_COUNTERS_CR",            /* 0x8AFA */
    "GL_SET_MAX_VIEWPORT_CR",                 /* 0x8AFB */
    "GL_HEAD_SPU_NAME_CR",                    /* 0x8AFC */
    "GL_PERF_GET_FRAME_DATA_CR",              /* 0x8AFD */
    "GL_PERF_GET_TIMER_DATA_CR",              /* 0x8AFE */
    "GL_PERF_DUMP_COUNTERS_CR",               /* 0x8AFF */
    "GL_PERF_SET_TOKEN_CR",                   /* 0x8B00 */
    "GL_PERF_SET_DUMP_ON_SWAP_CR",            /* 0x8B01 */
    "GL_PERF_SET_DUMP_ON_FINISH_CR",          /* 0x8B02 */
    "GL_PERF_SET_DUMP_ON_FLUSH_CR",           /* 0x8B03 */
    "GL_PERF_START_TIMER_CR",                 /* 0x8B04 */
    "GL_PERF_STOP_TIMER_CR",                  /* 0x8B05 */
    "GL_WINDOW_SIZE_CR",                      /* 0x8B06 */
    "GL_TILE_INFO_CR",                        /* 0x8B07 */
    "GL_GATHER_DRAWPIXELS_CR",                /* 0x8B08 */
    "GL_GATHER_PACK_CR",                      /* 0x8B09 */
    "GL_GATHER_CONNECT_CR",                   /* 0x8B0A */
    "GL_GATHER_POST_SWAPBUFFERS_CR",          /* 0x8B0B */
    "GL_SAVEFRAME_ENABLED_CR",                /* 0x8B0C */
    "GL_SAVEFRAME_FRAMENUM_CR",               /* 0x8B0D */
    "GL_SAVEFRAME_STRIDE_CR",                 /* 0x8B0E */
    "GL_SAVEFRAME_SINGLE_CR",                 /* 0x8B0F */
    "GL_SAVEFRAME_FILESPEC_CR",               /* 0x8B10 */
    "GL_READBACK_BARRIER_SIZE_CR",            /* 0x8B11 */
};

char * printspuEnumToStr(GLenum value)
{
    static char buf[16];
		if (value < 0x10000) {
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

