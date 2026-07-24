#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_bt.h>
#include <gui/gui.h>
#include <input/input.h>

// BLE Advertisement Data Types (GAP specification)
#define ADV_TYPE_FLAGS            0x01
#define ADV_TYPE_UUID128_COMPLETE 0x07
#define ADV_TYPE_NAME_COMPLETE    0x09

typedef struct {
    uint8_t data[31];
    uint8_t size;
} BleAdvPacket;

// Helper to construct a standard BLE advertising payload
static void build_adv_packet(
    BleAdvPacket* packet,
    const char* device_name,
    const uint8_t uuid128[16]) 
{
    packet->size = 0;

    // 1. Flags: General Discoverable Mode, BR/EDR Not Supported
    packet->data[packet->size++] = 0x02; // Length of this AD structure
    packet->data[packet->size++] = ADV_TYPE_FLAGS;
    packet->data[packet->size++] = 0x06;

    // 2. Complete Local Name
    uint8_t name_len = strlen(device_name);
    if(packet->size + name_len + 2 <= 31) {
        packet->data[packet->size++] = name_len + 1;
        packet->data[packet->size++] = ADV_TYPE_NAME_COMPLETE;
        memcpy(&packet->data[packet->size], device_name, name_len);
        packet->size += name_len;
    }

    // 3. 128-bit Custom Service UUID
    if(uuid128 != NULL && (packet->size + 18 <= 31)) {
        packet->data[packet->size++] = 17; // 16 bytes UUID + 1 byte Type
        packet->data[packet->size++] = ADV_TYPE_UUID128_COMPLETE;
        memcpy(&packet->data[packet->size], uuid128, 16);
        packet->size += 16;
    }
}

// GUI Render Callback
static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 15, "Custom BLE Beacon");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 10, 32, "Broadcasting UUID & Name");
    canvas_draw_str(canvas, 10, 50, "Press [BACK] to exit");
}

// Input Callback
static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FURI_WAIT_FOREVER);
}

int32_t custom_ble_adv_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Setup GUI
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // --- Configure BLE Payload ---
    // Example 128-bit UUID (in little-endian byte order as required by BLE spec)
    const uint8_t custom_uuid[16] = {
        0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
        0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21
    };

    BleAdvPacket packet;
    build_adv_packet(&packet, "FlipCustom", custom_uuid);

    // Configure Flipper BT Extra Beacon API
    GapExtraBeaconConfig config = {
        .min_interval = 100, // Advertising interval (ms)
        .max_interval = 150,
        .adv_channel_map = GapExtraBeaconChannelMapAll,
    };

    // Ensure Bluetooth HAL is active and start extra beacon
    furi_hal_bt_extra_beacon_set_config(&config);
    furi_hal_bt_extra_beacon_set_data(packet.data, packet.size);
    furi_hal_bt_extra_beacon_start();

    // Event Loop
    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, FURI_WAIT_FOREVER) == FuriStatusOk) {
            if(event.type == InputTypeShort && event.key == KeyBack) {
                running = false;
            }
        }
    }

    // Stop BLE Beacon and Cleanup Resources
    furi_hal_bt_extra_beacon_stop();

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);

    return 0;
}
