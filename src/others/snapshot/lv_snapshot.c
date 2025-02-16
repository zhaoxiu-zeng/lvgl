/**
 * @file lv_snapshot.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_snapshot.h"
#if LV_USE_SNAPSHOT

#include <stdbool.h>
#include "../../disp/lv_disp.h"
#include "../../core/lv_refr.h"
#include "../../disp/lv_disp_private.h"
#include "../../stdlib/lv_string.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/** Get the buffer needed for object snapshot image.
 *
 * @param obj    The object to generate snapshot.
 * @param cf     color format for generated image.
 *
 * @return the buffer size needed in bytes
 */
uint32_t lv_snapshot_buf_size_needed(lv_obj_t * obj, lv_color_format_t cf)
{
    LV_ASSERT_NULL(obj);
    switch(cf) {
        case LV_COLOR_FORMAT_RGB565:
        case LV_COLOR_FORMAT_RGB888:
        case LV_COLOR_FORMAT_XRGB8888:
        case LV_COLOR_FORMAT_ARGB8888:
            break;
        default:
            LV_LOG_WARN("Not supported color format");
            return 0;
    }

    lv_obj_update_layout(obj);

    /*Width and height determine snapshot image size.*/
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);
    lv_coord_t ext_size = _lv_obj_get_ext_draw_size(obj);
    w += ext_size * 2;
    h += ext_size * 2;
    uint8_t px_size;

    px_size = lv_color_format_get_size(cf);

    return w * h * px_size;
}

/** Take snapshot for object with its children, save image info to provided buffer.
 *
 * @param obj    The object to generate snapshot.
 * @param cf     color format for generated image.
 * @param dsc    image descriptor to store the image result.
 * @param buf    the buffer to store image data.
 * @param buff_size provided buffer size in bytes.
 *
 * @return LV_RES_OK on success, LV_RES_INV on error.
 */
lv_res_t lv_snapshot_take_to_buf(lv_obj_t * obj, lv_color_format_t cf, lv_img_dsc_t * dsc, void * buf,
                                 uint32_t buff_size)
{
    LV_ASSERT_NULL(obj);
    LV_ASSERT_NULL(dsc);
    LV_ASSERT_NULL(buf);

    switch(cf) {
        case LV_COLOR_FORMAT_RGB565:
        case LV_COLOR_FORMAT_RGB888:
        case LV_COLOR_FORMAT_XRGB8888:
        case LV_COLOR_FORMAT_ARGB8888:
            break;
        default:
            LV_LOG_WARN("Not supported color format");
            return LV_RES_INV;
    }

    if(lv_snapshot_buf_size_needed(obj, cf) > buff_size) return LV_RES_INV;

    /*Width and height determine snapshot image size.*/
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);
    lv_coord_t ext_size = _lv_obj_get_ext_draw_size(obj);
    w += ext_size * 2;
    h += ext_size * 2;

    lv_area_t snapshot_area;
    lv_obj_get_coords(obj, &snapshot_area);
    lv_area_increase(&snapshot_area, ext_size, ext_size);

    lv_memset(buf, 0x00, buff_size);
    lv_memzero(dsc, sizeof(lv_img_dsc_t));
    dsc->data = buf;
    dsc->header.w = w;
    dsc->header.h = h;
    dsc->header.cf = cf;

    lv_layer_t layer;
    lv_memzero(&layer, sizeof(layer));

    layer.color_format = dsc->header.cf;
    layer.buf_area = snapshot_area;
    layer.clip_area = snapshot_area;
    layer.buf = buf;

    lv_obj_redraw(&layer, obj);

    while(layer.draw_task_head) {
        lv_draw_dispatch_wait_for_request();
        lv_draw_dispatch_layer(NULL, &layer);
    }


    return LV_RES_OK;
}

/** Take snapshot for object with its children, alloc the memory needed.
 *
 * @param obj    The object to generate snapshot.
 * @param cf     color format for generated image.
 *
 * @return a pointer to an image descriptor, or NULL if failed.
 */
lv_img_dsc_t * lv_snapshot_take(lv_obj_t * obj, lv_color_format_t cf)
{
    LV_ASSERT_NULL(obj);
    uint32_t buff_size = lv_snapshot_buf_size_needed(obj, cf);

    void * buf = lv_malloc(buff_size);
    LV_ASSERT_MALLOC(buf);
    if(buf == NULL) {
        return NULL;
    }

    lv_img_dsc_t * dsc = lv_malloc(sizeof(lv_img_dsc_t));
    LV_ASSERT_MALLOC(buf);
    if(dsc == NULL) {
        lv_free(buf);
        return NULL;
    }

    if(lv_snapshot_take_to_buf(obj, cf, dsc, buf, buff_size) == LV_RES_INV) {
        lv_free(buf);
        lv_free(dsc);
        return NULL;
    }

    return dsc;
}

/** Free the snapshot image returned by @ref lv_snapshot_take
 *
 * It will firstly free the data image takes, then the image descriptor.
 *
 * @param dsc    The image descriptor generated by lv_snapshot_take.
 *
 */
void lv_snapshot_free(lv_img_dsc_t * dsc)
{
    if(!dsc)
        return;

    if(dsc->data)
        lv_free((void *)dsc->data);

    lv_free(dsc);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_SNAPSHOT*/
