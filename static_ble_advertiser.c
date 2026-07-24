#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
// ... other includes

#define TAG "StaticBLEAdv"
#define YOUR_STATIC_UUID 0x12345678-... // Define your 128-bit UUID bytes here

static void ble_advertiser_app(void* p) {
    // GUI setup (simple view showing status)
    // ...

    FuriHalBtState state = furi_hal_bt_get_state();
    if(state != FuriHalBtStateReady) {
        FURI_LOG_E(TAG, "Bluetooth not ready");
        return;
    }

    // Prepare advertising data with static UUID
    // Example: Service UUID in adv packet (adjust as needed)
    uint8_t adv_data[] = {
        // Flags
        0x02, 0x01, 0x06,
        // Complete 128-bit UUID
        0x11, 0x06, /* UUID bytes here, little-endian */
        // Name, manufacturer data, etc.
    };

    // For custom adv (may require firmware mods or specific builds)
    // furi_hal_bt_set_custom_adv_data(adv_data, sizeof(adv_data)); // if available

    furi_hal_bt_start_advertising();

    // Main loop with GUI updates, button handling to stop/start
    while(/* running */) {
        // Update GUI with "Advertising with UUID: XXX" 
        furi_delay_ms(100);
    }

    furi_hal_bt_stop_advertising();
}

int32_t static_ble_advertiser_app(void* p) {
    // Entry point
    ble_advertiser_app(p);
    return 0;
}
