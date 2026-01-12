#define UNIT_TEST
#include "mock_arduino.h"
#include "../src/modules/nfc/nfc.h"
#include <gtest/gtest.h>
#include <memory>
#include "mock_nfc_reader.h"

// Mock implementations
SPIClass SPI;
SerialClass Serial;

class NFCTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_reader = std::make_unique<MockNFCReader>();
        // Keep a raw pointer for test access
        reader_ptr = mock_reader.get();
        nfc = std::make_unique<NFC>(std::move(mock_reader));
    }

    void TearDown() override {
        nfc.reset();
    }

    std::unique_ptr<NFC> nfc;
    MockNFCReader* reader_ptr;
    std::unique_ptr<MockNFCReader> mock_reader;
};

TEST_F(NFCTest, InitializationTest) {
    nfc->begin();
    EXPECT_TRUE(reader_ptr->isInitialized());
    EXPECT_FALSE(reader_ptr->isPoweredDown());
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, ShutdownTest) {
    nfc->begin();
    nfc->shutdown();
    EXPECT_TRUE(reader_ptr->isPoweredDown());
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, CardDetectionTest) {
    bool callback_called = false;
    String detected_uid;
    
    nfc->begin();
    nfc->setCardDetectedCallback([&](const String& uid) {
        callback_called = true;
        detected_uid = uid;
    });

    reader_ptr->simulateCardPresent(true);
    nfc->update();

    EXPECT_TRUE(callback_called);
    EXPECT_EQ(detected_uid, "DEADBEEF");  // Default test UID
    EXPECT_FALSE(reader_ptr->isCardPresent());  // Card should be halted after reading
}

TEST_F(NFCTest, NoCardPresentTest) {
    bool callback_called = false;
    
    nfc->begin();
    nfc->setCardDetectedCallback([&](const String&) {
        callback_called = true;
    });

    reader_ptr->simulateCardPresent(false);
    nfc->update();

    EXPECT_FALSE(callback_called);
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, MultipleCardReadsTest) {
    int callback_count = 0;
    std::vector<String> detected_uids;
    
    nfc->begin();
    nfc->setCardDetectedCallback([&](const String& uid) {
        callback_count++;
        detected_uids.push_back(uid);
    });

    // First card
    reader_ptr->simulateCardPresent(true);
    nfc->update();
    
    // Change UID and simulate second card
    byte new_uid[] = {0x12, 0x34, 0x56, 0x78};
    reader_ptr->setTestUID(new_uid, 4);
    reader_ptr->simulateCardPresent(true);
    nfc->update();

    EXPECT_EQ(callback_count, 2);
    EXPECT_EQ(detected_uids[0], "DEADBEEF");
    EXPECT_EQ(detected_uids[1], "12345678");
}

TEST_F(NFCTest, CardRemovalTest) {
    nfc->begin();
    reader_ptr->simulateCardPresent(true);
    nfc->update();
    EXPECT_EQ(nfc->getState(), NFCState::Idle);  // Should return to idle after reading

    reader_ptr->simulateCardPresent(false);
    nfc->update();
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, PowerCycleTest) {
    nfc->begin();
    reader_ptr->simulateCardPresent(true);
    nfc->update();
    
    nfc->shutdown();
    EXPECT_FALSE(reader_ptr->isNewCardPresent());
    
    nfc->begin();
    EXPECT_TRUE(reader_ptr->isInitialized());
    reader_ptr->simulateCardPresent(true);
    EXPECT_TRUE(reader_ptr->isNewCardPresent());
}

TEST_F(NFCTest, CallbackUpdateTest) {
    std::vector<String> detected_uids;
    
    nfc->begin();
    nfc->setCardDetectedCallback([&](const String& uid) {
        detected_uids.push_back(uid);
    });

    reader_ptr->simulateCardPresent(true);
    nfc->update();
    
    // Update callback
    nfc->setCardDetectedCallback([&](const String& uid) {
        detected_uids.push_back("NEW_" + uid);
    });

    reader_ptr->simulateCardPresent(true);
    nfc->update();

    EXPECT_EQ(detected_uids.size(), 2);
    EXPECT_EQ(detected_uids[0], "DEADBEEF");
    EXPECT_EQ(detected_uids[1], "NEW_DEADBEEF");
}

TEST_F(NFCTest, InitializationFailureTest) {
    reader_ptr->setInitializationShouldFail(true);
    EXPECT_THROW(nfc->begin(), std::runtime_error);
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, InvalidUIDTest) {
    bool callback_called = false;
    nfc->begin();
    nfc->setCardDetectedCallback([&](const String&) {
        callback_called = true;
    });

    // Set invalid UID
    reader_ptr->setTestUID(nullptr, 0);
    reader_ptr->simulateCardPresent(true);
    nfc->update();

    EXPECT_FALSE(callback_called);
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, OversizedUIDTest) {
    std::string received_uid;
    nfc->begin();
    nfc->setCardDetectedCallback([&](const String& uid) {
        received_uid = uid.c_str();
    });

    // Set oversized UID
    byte large_uid[15] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    reader_ptr->setTestUID(large_uid, 15);
    reader_ptr->simulateCardPresent(true);
    nfc->update();

    // Should truncate to 10 bytes
    EXPECT_EQ(received_uid.length(), 20);  // 10 bytes = 20 hex chars
}

TEST_F(NFCTest, UninitializedOperationsTest) {
    // Don't call begin(), try operations on uninitialized reader
    reader_ptr->simulateCardPresent(true);
    nfc->update();
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, ConcurrentOperationsTest) {
    nfc->begin();
    reader_ptr->simulateCardPresent(true);

    // Simulate rapid updates
    for (int i = 0; i < 10; i++) {
        nfc->update();
    }

    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, CallbackExceptionTest) {
    nfc->begin();
    nfc->setCardDetectedCallback([](const String&) {
        throw std::runtime_error("Callback error");
    });

    reader_ptr->simulateCardPresent(true);
    EXPECT_NO_THROW(nfc->update());  // NFC should handle callback exceptions
    EXPECT_EQ(nfc->getState(), NFCState::Idle);
}

TEST_F(NFCTest, StateTransitionStressTest) {
    nfc->begin();
    std::vector<NFCState> state_history;

    // Rapid state changes
    for (int i = 0; i < 100; i++) {
        reader_ptr->simulateCardPresent(i % 2 == 0);
        nfc->update();
        state_history.push_back(nfc->getState());
    }

    // Verify no invalid states
    for (const auto& state : state_history) {
        EXPECT_TRUE(state == NFCState::Idle || state == NFCState::Reading);
    }
}

TEST_F(NFCTest, PowerCycleStressTest) {
    for (int i = 0; i < 10; i++) {
        nfc->begin();
        EXPECT_TRUE(reader_ptr->isInitialized());
        
        reader_ptr->simulateCardPresent(true);
        nfc->update();
        
        nfc->shutdown();
        EXPECT_TRUE(reader_ptr->isPoweredDown());
    }
} 