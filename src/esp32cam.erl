%%
%% Copyright (c) 2020 dushin.net
%% All rights reserved.
%%
%% Licensed under the Apache License, Version 2.0 (the "License");
%% you may not use this file except in compliance with the License.
%% You may obtain a copy of the License at
%%
%%     http://www.apache.org/licenses/LICENSE-2.0
%%
%% Unless required by applicable law or agreed to in writing, software
%% distributed under the License is distributed on an "AS IS" BASIS,
%% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
%% See the License for the specific language governing permissions and
%% limitations under the License.
%%
-module(esp32cam).

-export([init/0, init/1, capture/0, capture/1]).

-type esp32cam_config_item() ::
    {model, aithinker}
    | {frame_size, qvga|cif|vga|svga|xga|sxga|uxga}
    | {jpeg_quality, 0..63}.
-type esp32cam_config() :: [esp32cam_config_item()].

-type capture_param() :: {flash, on|off}.
-type capture_params() :: [capture_param()].
-type image() :: binary().

-define(DEFAULT_CONFIG, [{model, aithinker}, {frame_size, xga}, {jpeg_quality, 12}]).

%%-----------------------------------------------------------------------------
%% @returns `ok' or error with reason
%% @doc     Initialize the camera.
%%          Use this function to initialize the ESP32 camera with default configuration.
%% @end
%%-----------------------------------------------------------------------------
-spec init() -> ok | {error, Reason::term()}.
init() ->
    init(?DEFAULT_CONFIG).

%%-----------------------------------------------------------------------------
%% @param   Config  the ESP32cam configuration
%% @returns `ok' or error with reason
%% @doc     Initialize the camera.
%%          Use this function to initialize the ESP32 camera
%% @end
%%-----------------------------------------------------------------------------
-spec init(Config::esp32cam_config()) -> ok | {error, Reason::term()}.
init(_Config) ->
    throw(nif_error).

%%-----------------------------------------------------------------------------
%% @returns The image data (e.g., JPEG).
%% @doc     Capture an image with the camera.
%%          Use this function to capture an image using the ESP32 camera
%% @end
%%-----------------------------------------------------------------------------
-spec capture() -> {ok, image()} | {error, Reason::term()}.
capture() ->
    throw(nif_error).

%%-----------------------------------------------------------------------------
%% @param   CaptureParams capture parameters
%% @returns The image data (e.g., JPEG).
%% @doc     Capture an image with the camera.
%%          Use this function to capture an image using the ESP32 camera
%% @end
%%-----------------------------------------------------------------------------
-spec capture(CaptureParams::capture_params()) -> {ok, image()} | {error, Reason::term()}.
capture(_CaptureParams) ->
    throw(nif_error).
