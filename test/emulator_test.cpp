#define CONFIG_CATCH_MAIN
#include "catch2/catch_all.hpp"
#include "../src/CPU.h"

CPU cpu;

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

class testRunListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const&) override {
    }
};

CATCH_REGISTER_LISTENER(testRunListener)
TEST_CASE( "Test CPU", "[single-file]" ) {
    cpu.init();
    cpu.signals.CA_SE = 1;
    cpu.signals.HLT = 0;
    REQUIRE(sizeof(cpu.signals) == 4);
    REQUIRE(cpu.signals.val == 0x400000);
}
