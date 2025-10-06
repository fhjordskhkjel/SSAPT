// SSAPT Example Application
// Demonstrates how to use the screenshot blocking driver

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "driver.h"

void ShowProtectionStatus() {
    if (IsBlockingEnabled()) {
        std::cout << "\n[PROTECTION] Screenshot blocking is ACTIVE" << std::endl;
        std::cout << "Try taking a screenshot now - it should be blocked!" << std::endl;
    } else {
        std::cout << "\n[PROTECTION] Screenshot blocking is INACTIVE" << std::endl;
        std::cout << "Screenshots are currently allowed." << std::endl;
    }
}

void DemoBasicUsage() {
    std::cout << "\n=== Basic Usage Demo ===" << std::endl;
    
    // Enable blocking
    std::cout << "\nEnabling screenshot protection..." << std::endl;
    EnableBlocking();
    ShowProtectionStatus();
    
    std::cout << "\nWaiting 5 seconds (try to take a screenshot now)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Disable blocking
    std::cout << "\nDisabling screenshot protection..." << std::endl;
    DisableBlocking();
    ShowProtectionStatus();
    
    std::cout << "\nWaiting 5 seconds (screenshots should work now)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void DemoToggleProtection() {
    std::cout << "\n=== Toggle Protection Demo ===" << std::endl;
    
    for (int i = 0; i < 3; i++) {
        std::cout << "\nToggle #" << (i + 1) << std::endl;
        
        if (IsBlockingEnabled()) {
            std::cout << "Currently: PROTECTED - Disabling..." << std::endl;
            DisableBlocking();
        } else {
            std::cout << "Currently: UNPROTECTED - Enabling..." << std::endl;
            EnableBlocking();
        }
        
        ShowProtectionStatus();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void DemoSecureOperation() {
    std::cout << "\n=== Secure Operation Demo ===" << std::endl;
    std::cout << "Simulating viewing of sensitive content..." << std::endl;
    
    // Enable protection before showing sensitive content
    EnableBlocking();
    std::cout << "\n[SECURE MODE] Protection enabled" << std::endl;
    std::cout << "Displaying sensitive information for 8 seconds..." << std::endl;
    std::cout << "\n======================================" << std::endl;
    std::cout << "  CONFIDENTIAL INFORMATION" << std::endl;
    std::cout << "  User: admin@example.com" << std::endl;
    std::cout << "  API Key: sk_live_123456789abcdef" << std::endl;
    std::cout << "  Token: eyJhbGciOiJIUzI1NiIsInR5cCI" << std::endl;
    std::cout << "======================================" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(8));
    
    // Disable protection after sensitive content is gone
    DisableBlocking();
    std::cout << "\n[SECURE MODE] Protection disabled - content cleared" << std::endl;
}

void ShowMenu() {
    std::cout << "\n=== SSAPT Example Application ===" << std::endl;
    std::cout << "1. Basic Usage Demo" << std::endl;
    std::cout << "2. Toggle Protection Demo" << std::endl;
    std::cout << "3. Secure Operation Demo" << std::endl;
    std::cout << "4. Manual Control" << std::endl;
    std::cout << "5. Exit" << std::endl;
    std::cout << "\nChoose an option: ";
}

void ManualControl() {
    std::cout << "\n=== Manual Control Mode ===" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  1 - Enable protection" << std::endl;
    std::cout << "  0 - Disable protection" << std::endl;
    std::cout << "  s - Show status" << std::endl;
    std::cout << "  q - Quit manual mode" << std::endl;
    
    char cmd;
    while (true) {
        std::cout << "\nCommand: ";
        std::cin >> cmd;
        
        switch (cmd) {
            case '1':
                EnableBlocking();
                std::cout << "Protection ENABLED" << std::endl;
                break;
            case '0':
                DisableBlocking();
                std::cout << "Protection DISABLED" << std::endl;
                break;
            case 's':
                ShowProtectionStatus();
                break;
            case 'q':
                return;
            default:
                std::cout << "Unknown command" << std::endl;
        }
    }
}

int main() {
    std::cout << "SSAPT (Screenshot Anti-Protection Testing) Example" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    // Initial state
    ShowProtectionStatus();
    
    int choice;
    while (true) {
        ShowMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                DemoBasicUsage();
                break;
            case 2:
                DemoToggleProtection();
                break;
            case 3:
                DemoSecureOperation();
                break;
            case 4:
                ManualControl();
                break;
            case 5:
                std::cout << "\nExiting..." << std::endl;
                DisableBlocking(); // Ensure blocking is disabled on exit
                return 0;
            default:
                std::cout << "Invalid option" << std::endl;
        }
    }
    
    return 0;
}
