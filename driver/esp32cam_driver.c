//
// Copyright (c) dushin.net
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <esp_camera.h>
#include <esp_log.h>

#include <stdlib.h>

#include "context.h"
#include "defaultatoms.h"
#include "interop.h"
#include "nifs.h"
#include "port.h"
#include "term.h"

//#define ENABLE_TRACE
#include "trace.h"

#define TAG "esp32cam"
#define DEFAULT_JPEG_QUALITY 12
#define INVALID_JPEG_QUALITY -1

// AI-THINKER
#define AI_THINKER_CAM_PIN_PWDN     32
#define AI_THINKER_CAM_PIN_RESET    -1
#define AI_THINKER_CAM_PIN_XCLK      0
#define AI_THINKER_CAM_PIN_SIOD     26
#define AI_THINKER_CAM_PIN_SIOC     27

#define AI_THINKER_CAM_PIN_D7       35
#define AI_THINKER_CAM_PIN_D6       34
#define AI_THINKER_CAM_PIN_D5       39
#define AI_THINKER_CAM_PIN_D4       36
#define AI_THINKER_CAM_PIN_D3       21
#define AI_THINKER_CAM_PIN_D2       19
#define AI_THINKER_CAM_PIN_D1       18
#define AI_THINKER_CAM_PIN_D0        5
#define AI_THINKER_CAM_PIN_VSYNC    25
#define AI_THINKER_CAM_PIN_HREF     23
#define AI_THINKER_CAM_PIN_PCLK     22


static const char *const frame_size_a =       "\xA"  "frame_size";
static const char *const jpeg_quality_a =     "\xC"  "jpeg_quality";
static const char *const qvga_a =             "\x4"  "qvga";
static const char *const cif_a =              "\x3"  "cif";
static const char *const vga_a =              "\x3"  "vga";
static const char *const svga_a =             "\x4"  "svga";
static const char *const xga_a =              "\x3"  "xga";
static const char *const sxga_a =             "\x4"  "sxga";
static const char *const uxga_a =             "\x4"  "uxga";
static const char *const flash_a =            "\x5"  "flash";
static const char *const bad_state_a =        "\x9"  "bad_state";
static const char *const capture_failed_a =   "\xE"  "capture_failed";
//                                                    123456789ABCDEF

uint8_t camera_initialized = 0;


static framesize_t get_frame_size(Context *ctx, term frame_size)
{
    if (term_is_nil(frame_size)) {
        return FRAMESIZE_XGA;
    } else if (frame_size == context_make_atom(ctx, qvga_a)) {
        return FRAMESIZE_QVGA;
    } else if (frame_size == context_make_atom(ctx, cif_a)) {
        return FRAMESIZE_CIF;
    } else if (frame_size == context_make_atom(ctx, vga_a)) {
        return FRAMESIZE_VGA;
    } else if (frame_size == context_make_atom(ctx, svga_a)) {
        return FRAMESIZE_SVGA;
    } else if (frame_size == context_make_atom(ctx, xga_a)) {
        return FRAMESIZE_XGA;
    } else if (frame_size == context_make_atom(ctx, sxga_a)) {
        return FRAMESIZE_SXGA;
    } else if (frame_size == context_make_atom(ctx, uxga_a)) {
        return FRAMESIZE_UXGA;
    } else {
        return FRAMESIZE_INVALID;
    }
}


static framesize_t get_jpeg_quality(term jpeg_quality)
{
    if (term_is_nil(jpeg_quality)) {
        return DEFAULT_JPEG_QUALITY;
    } else if (term_is_any_integer(jpeg_quality)) {
        uint32_t val = term_to_int(jpeg_quality);
        if (val > 63) {
            return INVALID_JPEG_QUALITY;
        } else {
            return val;
        }
    } else {
        return INVALID_JPEG_QUALITY;
    }
}

static camera_config_t *create_camera_config(framesize_t frame_size, int jpeg_quality)
{
    camera_config_t *config = malloc(sizeof(camera_config_t));
    if (IS_NULL_PTR(config)) {
        ESP_LOGE(TAG, "Memory allocation failed");
        return NULL;
    }
    config->pin_pwdn  = AI_THINKER_CAM_PIN_PWDN;
    config->pin_reset = AI_THINKER_CAM_PIN_RESET;
    config->pin_xclk = AI_THINKER_CAM_PIN_XCLK;
    config->pin_sscb_sda = AI_THINKER_CAM_PIN_SIOD;
    config->pin_sscb_scl = AI_THINKER_CAM_PIN_SIOC;
    config->pin_d7 = AI_THINKER_CAM_PIN_D7;
    config->pin_d6 = AI_THINKER_CAM_PIN_D6;
    config->pin_d5 = AI_THINKER_CAM_PIN_D5;
    config->pin_d4 = AI_THINKER_CAM_PIN_D4;
    config->pin_d3 = AI_THINKER_CAM_PIN_D3;
    config->pin_d2 = AI_THINKER_CAM_PIN_D2;
    config->pin_d1 = AI_THINKER_CAM_PIN_D1;
    config->pin_d0 = AI_THINKER_CAM_PIN_D0;
    config->pin_vsync = AI_THINKER_CAM_PIN_VSYNC;
    config->pin_href = AI_THINKER_CAM_PIN_HREF;
    config->pin_pclk = AI_THINKER_CAM_PIN_PCLK;
    config->xclk_freq_hz = 20000000;
    config->ledc_timer = LEDC_TIMER_0;
    config->ledc_channel = LEDC_CHANNEL_0;
    config->pixel_format = PIXFORMAT_JPEG;
    config->frame_size = frame_size;
    config->jpeg_quality = jpeg_quality;
    config->fb_count = 1;

    return config;
}


static term nif_esp32cam_init(Context *ctx, int argc, term argv[])
{
    term config;
    if (argc == 0) {
        config = term_nil();
    } else {
        VALIDATE_VALUE(argv[0], term_is_list);
        config = argv[0];
    }

    framesize_t frame_size = get_frame_size(ctx, interop_proplist_get_value(config, context_make_atom(ctx, frame_size_a)));
    if (frame_size == FRAMESIZE_INVALID) {
        RAISE_ERROR(BADARG_ATOM);
    }
    int jpeg_quality = get_jpeg_quality(interop_proplist_get_value(config, context_make_atom(ctx, jpeg_quality_a)));
    if (jpeg_quality == INVALID_JPEG_QUALITY) {
        RAISE_ERROR(BADARG_ATOM);
    }

    camera_config_t *camera_config = create_camera_config(frame_size, jpeg_quality);
    if (IS_NULL_PTR(camera_config)) {
        RAISE_ERROR(MEMORY_ATOM);
    }

    esp_err_t err = esp_camera_init(camera_config);
    free(camera_config);
    if (err != ESP_OK) {
        if (UNLIKELY(memory_ensure_free(ctx, 3) != MEMORY_GC_OK)) {
            RAISE_ERROR(MEMORY_ATOM);
        }
        term error = port_create_error_tuple(ctx, term_from_int32(err));
        return error;
    }
    camera_initialized = 1;
    return OK_ATOM;
}


static term nif_esp32cam_capture(Context *ctx, int argc, term argv[])
{
    term params;
    if (argc == 0) {
        params = term_nil();
    } else {
        VALIDATE_VALUE(argv[0], term_is_list);
        params = argv[0];
    }

    if (!camera_initialized) {
        ESP_LOGE(TAG, "Camera not initialized!  Bad state.");
        RAISE_ERROR(context_make_atom(ctx, bad_state_a));
    }

    uint8_t use_flash = 0;


    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        if (UNLIKELY(memory_ensure_free(ctx, 3) != MEMORY_GC_OK)) {
            RAISE_ERROR(MEMORY_ATOM);
        }
        term error = port_create_error_tuple(ctx, context_make_atom(ctx, capture_failed_a));
        return error;
    }

    if (UNLIKELY(memory_ensure_free(ctx, term_binary_data_size_in_terms(fb->len) + BINARY_HEADER_SIZE + 4) != MEMORY_GC_OK)) {
        esp_camera_fb_return(fb);
        ESP_LOGE(TAG, "Image memory allocation (%i) failed", fb->len);
        RAISE_ERROR(MEMORY_ATOM);
    }
    term image = term_from_literal_binary((const char *)fb->buf, fb->len, ctx);
    esp_camera_fb_return(fb);

    return port_create_tuple2(ctx, OK_ATOM, image);
}


static const struct Nif esp32cam_init_nif =
{
    .base.type = NIFFunctionType,
    .nif_ptr = nif_esp32cam_init
};
static const struct Nif esp32cam_capture_nif =
{
    .base.type = NIFFunctionType,
    .nif_ptr = nif_esp32cam_capture
};


const struct Nif *esp32cam_driver_nifs_get_nif(const char *nifname)
{
    if (strcmp("esp32cam:init/0", nifname) == 0) {
        TRACE("Resolved platform nif %s ...\n", nifname);
        return &esp32cam_init_nif;
    }
    if (strcmp("esp32cam:init/1", nifname) == 0) {
        TRACE("Resolved platform nif %s ...\n", nifname);
        return &esp32cam_init_nif;
    }
    if (strcmp("esp32cam:capture/0", nifname) == 0) {
        TRACE("Resolved platform nif %s ...\n", nifname);
        return &esp32cam_capture_nif;
    }
    if (strcmp("esp32cam:capture/1", nifname) == 0) {
        TRACE("Resolved platform nif %s ...\n", nifname);
        return &esp32cam_capture_nif;
    }

    return NULL;
}
