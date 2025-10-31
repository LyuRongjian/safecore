# SafeCore Framework

![Version](https://img.shields.io/badge/version-0.0.4-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-embedded-orange.svg)

**SafeCore** is a lightweight, modular, and safety-critical framework designed for embedded systems. It provides a robust foundation for building reliable real-time applications with hierarchical state machines, event-driven architecture, and comprehensive safety mechanisms.

## 🌟 Key Features

- **Hierarchical State Machine (HSM)**: Robust implementation with nested states, entry/exit actions, and deterministic event processing
- **Event Bus System**: Publish-subscribe mechanism for decoupled inter-component communication
- **Priority Queue Management**: Multi-level priority queues for critical event handling
- **Event Filtering**: Configurable rule-based event filtering system
- **Safety Mechanisms**: Watchdog integration, self-testing, and safe state management
- **Diagnostics System**: Automotive-grade fault detection and DTC (Diagnostic Trouble Code) management
- **Communication Bridge**: Protocol abstraction layer supporting CAN and other communication protocols
- **AUTOSAR Compatibility**: Optional AUTOSAR BSW (Basic Software) compatibility layer
- **Modular Design**: Enable/disable features based on your requirements
- **Lightweight**: Minimal memory footprint suitable for resource-constrained systems

## 📋 Table of Contents

- [Architecture Overview](#architecture-overview)
- [Getting Started](#getting-started)
- [Configuration](#configuration)
- [Core Modules](#core-modules)
- [Usage Examples](#usage-examples)
- [API Reference](#api-reference)
- [Safety Considerations](#safety-considerations)
- [Contributing](#contributing)
- [License](#license)

## 🏗️ Architecture Overview

SafeCore is built on a modular architecture with the following core components:

```
┌─────────────────────────────────────────────────────┐
│                  Application Layer                  │
├─────────────────────────────────────────────────────┤
│  State Machine  │  Event Bus  │  Safety Management  │
├─────────────────────────────────────────────────────┤
│ Priority Queue  │   Filters   │    Diagnostics      │
├─────────────────────────────────────────────────────┤
│          Communication Bridge (CAN, etc.)           │
├─────────────────────────────────────────────────────┤
│              Platform Abstraction Layer             │
└─────────────────────────────────────────────────────┘
```

## 🚀 Getting Started

### Prerequisites

- C compiler (GCC, Clang, or embedded toolchain)
- Standard C library support
- Target platform: ARM Cortex-M, x86, or other embedded architectures

### Basic Integration

1. **Include SafeCore headers in your project:**

```c
#include "safecore_core.h"
#include "safecore_safety.h"
```

2. **Implement platform-specific functions:**

```c
uint32_t safecore_get_tick_ms(void) {
    // Return system tick in milliseconds
    return HAL_GetTick();
}

void safecore_error_handler(const char *msg) {
    // Handle critical errors
    printf("ERROR: %s\n", msg);
}
```

3. **Initialize SafeCore framework:**

```c
int main(void) {
    // Initialize safety mechanisms
    sc_safety_init();
    
    // Initialize event bus
    sc_init();
    
    // Your application code
    while (1) {
        sc_process();
        // Application logic
    }
}
```

## ⚙️ Configuration

SafeCore is highly configurable through `safecore_config.h`. Key configuration options:

### Framework Version
```c
#define SAFECORE_VERSION_MAJOR    0
#define SAFECORE_VERSION_MINOR    0
#define SAFECORE_VERSION_PATCH    4
```

### Core Features
```c
#define SAFECORE_BASIC_ENABLED               1   /* Basic event bus + state machine */
#define SAFECORE_MAX_HSM_DEPTH               4   /* HSM maximum nesting depth */
#define SAFECORE_EVENT_QUEUE_SIZE            32  /* Event queue size (power of 2) */
#define SAFECORE_MAX_SUBSCRIBERS             8   /* Maximum event subscribers */
```

### Priority System
```c
#define SAFECORE_PRIORITY_ENABLED            1   /* Enable priority queues */
#define SAFECORE_EVENT_PRIORITIES            3   /* Number of priority levels */
#define SAFECORE_EMERGENCY_PRIORITY          0   /* Emergency events */
#define SAFECORE_STANDARD_PRIORITY           1   /* Standard events */
#define SAFECORE_LOW_PRIORITY                2   /* Low priority events */
```

### Safety & Diagnostics
```c
#define SAFECORE_SAFETY_ENABLED              1   /* Safety mechanisms */
#define SAFECORE_DIAGNOSTICS_ENABLED         1   /* Diagnostics system */
#define SAFECORE_MAX_DTCS                    128 /* Maximum DTC count */
```

### Communication
```c
#define SAFECORE_COMM_ENABLED                1   /* Communication bridge */
#define SAFECORE_COMM_CAN_ENABLED            1   /* CAN protocol support */
```

## 🧩 Core Modules

### 1. State Machine (`safecore_core.h`)

Hierarchical state machine with deterministic event processing:

```c
sc_state_machine_t sm;
sc_sm_init(&sm, top_state_handler, context, "MyStateMachine");
sc_sm_dispatch(&sm, &event);
```

**Features:**
- Nested state hierarchies (configurable depth)
- Entry and exit actions
- Event-driven transitions
- User-defined context per state

### 2. Event Bus (`safecore_core.h`)

Publish-subscribe event system for component communication:

```c
// Subscribe to events
sc_subscribe(EVENT_SENSOR_DATA, sensor_callback, context);

// Publish events
sc_publish(EVENT_SENSOR_DATA, &data, sizeof(data));

// Process events
sc_process();
```

### 3. Priority Queue (`safecore_priority.h`)

Multi-level priority queues for critical event handling:

```c
// Publish with priority
sc_publish_emergency(&critical_event, sizeof(critical_event));
sc_publish_standard(&normal_event, sizeof(normal_event));

// Process prioritized events
sc_priority_process();
```

### 4. Event Filters (`safecore_filters.h`)

Rule-based event filtering:

```c
sc_filter_rule_t rule = {
    .event_id = EVENT_SENSOR_DATA,
    .action = SAFECORE_FILTER_ALLOW,
    .priority = 1
};
sc_filters_add_rule(&rule);
```

### 5. Safety Module (`safecore_safety.h`)

Safety-critical features for reliable operation:

```c
// Initialize safety system
sc_safety_init();

// Periodic safety checks
sc_safety_self_test();

// Watchdog management
sc_safety_kick_watchdog();

// Safe state handling
sc_safety_enter_safe_state();
```

### 6. Diagnostics (`safecore_diagnostics.h`)

Automotive-grade diagnostic system:

```c
// Report diagnostic trouble code
sc_diag_report_dtc(DTC_SENSOR_FAULT, DIAG_LEVEL_ERROR);

// Query diagnostics
sc_diag_status_t status = sc_diag_get_status();

// Clear DTCs
sc_diag_clear_all_dtcs();
```

### 7. Communication Bridge (`safecore_com_bridge.h`)

Protocol abstraction for communication systems:

```c
// Initialize communication
sc_com_init();

// Send CAN frame
sc_can_frame_type_t frame = {
    .id = 0x123,
    .data = {0x01, 0x02, 0x03},
    .dlc = 3
};
sc_com_can_send_frame(&frame);
```

## 💡 Usage Examples

### Example 1: Basic State Machine

```c
sc_sm_result_t idle_state(sc_state_machine_t *sm, const sc_sm_event_t *e, void *ctx) {
    switch (e->id) {
        case SM_EVENT_ENTRY:
            printf("Entering IDLE state\n");
            return SM_HANDLED;
            
        case EVENT_START:
            printf("Starting...\n");
            // Transition to running state
            return SM_TRANSITION;
            
        case SM_EVENT_EXIT:
            printf("Exiting IDLE state\n");
            return SM_HANDLED;
            
        default:
            return SM_UNHANDLED;
    }
}
```

### Example 2: Event-Driven Communication

```c
void sensor_callback(const uint8_t *data, size_t len, void *ctx) {
    sensor_data_t *sensor = (sensor_data_t *)data;
    printf("Sensor value: %d\n", sensor->value);
    
    // Process sensor data
    if (sensor->value > THRESHOLD) {
        sc_publish_emergency(&alarm_event, sizeof(alarm_event));
    }
}

// Subscribe to sensor events
sc_subscribe(EVENT_SENSOR_DATA, sensor_callback, NULL);
```

### Example 3: Priority-Based Event Processing

```c
// Emergency event (highest priority)
sc_publish_emergency(&critical_fault, sizeof(critical_fault));

// Standard event
sc_publish_standard(&normal_operation, sizeof(normal_operation));

// Low priority event
sc_publish_low(&background_task, sizeof(background_task));

// Process all queues (emergency first)
sc_priority_process();
```

## 📚 API Reference

### Core Functions

| Function | Description |
|----------|-------------|
| `sc_init()` | Initialize SafeCore framework |
| `sc_process()` | Process pending events in event bus |
| `sc_subscribe()` | Subscribe to specific event type |
| `sc_publish()` | Publish event to event bus |
| `sc_sm_init()` | Initialize state machine |
| `sc_sm_dispatch()` | Dispatch event to state machine |

### Safety Functions

| Function | Description |
|----------|-------------|
| `sc_safety_init()` | Initialize safety mechanisms |
| `sc_safety_self_test()` | Perform system self-test |
| `sc_safety_kick_watchdog()` | Reset watchdog timer |
| `sc_safety_enter_safe_state()` | Enter safe operating state |
| `sc_safety_get_status()` | Get current safety status |

### Diagnostic Functions

| Function | Description |
|----------|-------------|
| `sc_diag_report_dtc()` | Report diagnostic trouble code |
| `sc_diag_get_status()` | Get diagnostic status |
| `sc_diag_clear_all_dtcs()` | Clear all trouble codes |

## 🛡️ Safety Considerations

SafeCore is designed with safety-critical applications in mind:

1. **Deterministic Behavior**: All operations have bounded execution time
2. **Memory Safety**: No dynamic memory allocation; all buffers are statically sized
3. **Watchdog Integration**: Built-in watchdog support for system monitoring
4. **Error Handling**: Comprehensive error detection and safe shutdown mechanisms
5. **Self-Testing**: Periodic self-test capabilities for fault detection
6. **Priority Management**: Critical events are processed with highest priority

### Safety Certifications

While SafeCore itself is not certified, it has been designed following principles from:
- IEC 61508 (Functional Safety)
- ISO 26262 (Automotive Safety)
- MISRA C guidelines

## 🔧 Platform Porting

To port SafeCore to your platform, implement these functions in `safecore_port.h`:

```c
/* Time management */
uint32_t safecore_get_tick_ms(void);

/* Error handling */
void safecore_error_handler(const char *msg);

/* Optional: Watchdog */
void safecore_watchdog_init(void);
void safecore_watchdog_kick(void);

/* Optional: Critical sections */
void safecore_enter_critical(void);
void safecore_exit_critical(void);
```

## 📊 Memory Footprint

Typical memory usage (with all features enabled):

- **Code Size**: ~15-25 KB
- **RAM Usage**: ~2-8 KB (depending on configuration)
- **Stack Usage**: ~1-2 KB per state machine

## 🧪 Testing

The `main.c` file includes comprehensive examples demonstrating all framework features. To build and run tests:

```bash
gcc main.c safecore_*.c -o safecore_test
./safecore_test
```

## 🤝 Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style
- Follow MISRA C guidelines where applicable
- Use consistent indentation (4 spaces)
- Document all public APIs with Doxygen-style comments
- Keep functions focused and under 100 lines

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙋 Support

For questions, issues, or feature requests:
- Open an issue on GitHub
- Contact: [Your Contact Information]

## 🗺️ Roadmap

- [ ] Enhanced AUTOSAR compliance
- [ ] Additional communication protocol support (LIN, FlexRay)
- [ ] RTOS integration examples
- [ ] Advanced logging and tracing
- [ ] Memory pool management
- [ ] Safety certification artifacts

## 🙏 Acknowledgments

SafeCore is designed for embedded systems developers who need reliable, deterministic, and safety-critical event processing frameworks.

---

**Version**: 0.0.4
**Last Updated**: October 31, 2025  
**Maintained by**: LyuRongjian
