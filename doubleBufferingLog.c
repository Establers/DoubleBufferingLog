#include <stdbool.h>
#include <stdint.h>  // uint8_t와 uint32_t 타입을 위한 헤더

#define BUFFER_SIZE 1000

#define MARKER_SIZE 4
#define END_MARKER 0xABCDEEEE
#define START_MARKER 0x0213E0E0

volatile int timer_ms = 0; // 1ms마다 증가하는 타이머 변수

uint8_t bufferA[BUFFER_SIZE];
uint8_t bufferB[BUFFER_SIZE];
int indexA = 0, indexB = 0;
uint8_t *activeBuffer = bufferA;
uint8_t *transmitBuffer = bufferB;
int *activeIndex = &indexA;
int *transmitIndex = &indexB;

bool isTransmitting = false;
bool readyToSend = false;
bool timer_active = true;

void swapBuffers(void) {
    uint8_t *tempBuffer = activeBuffer;
    activeBuffer = transmitBuffer;
    transmitBuffer = tempBuffer;

    int *tempIndex = activeIndex;
    activeIndex = transmitIndex;
    transmitIndex = tempIndex;
}

void appendEndMarkerToTransmitBuffer(void) {
    int endIndex = MARKER_SIZE + *transmitIndex;
    memcpy(transmitBuffer + endIndex, &END_MARKER, sizeof(endMarker));
    *transmitIndex = endIndex + 4;
}

void initializeActiveBuffer(void) {
    memcpy(activeBuffer, &START_MARKER, sizeof(startMarker));
    *activeIndex = MARKER_SIZE;  // Start index after the start marker
}

void switchBuffers(void) {
    if (!isTransmitting) {
        swapBuffers();
        appendEndMarkerToTransmitBuffer();
        initializeActiveBuffer();
    }
}

void logData(uint32_t data) {
    if (*activeIndex <= BUFFER_SIZE - 12) {
        activeBuffer[(*activeIndex)++] = (data >> 24) & 0xFF;
        activeBuffer[(*activeIndex)++] = (data >> 16) & 0xFF;
        activeBuffer[(*activeIndex)++] = (data >> 8 ) & 0xFF;
        activeBuffer[(*activeIndex)++] = (data >> 0 ) & 0xFF;
    }
}

// 1ms timer 내에서 실행되어야 할 함수
void timer_interrupt_1ms(void) {
    if (timer_ms < 100 && timer_active) {
        timer_ms++; // 1ms마다 증가
    }

    if (timer_ms >= 100 && !isTransmitting && readyToSend) {
        uartSend(); // 데이터 전송 시작
        readyToSend = false;
        timer_active = false;  // 타이머 비활성화
    }
}

void uartSend(void) {
    if (*transmitIndex > 0 && !isTransmitting) {  // 전송할 데이터가 있는 경우에만 전송 시작
        isTransmitting = true;
        SCI1.TDR = transmitBuffer[0];  // 첫 바이트 전송
        SCI1.SCR.BIT.TIE = 1;  // Transmit Interrupt Enable 설정 (필요시)
    }
}

// txi
void __attribute__((interrupt)) sci1_txi_isr(void) {
    if(specific_logic) {
        static int txIndex = 1;
        if (txIndex < *transmitIndex) {
            SCI1.TDR = transmitBuffer[txIndex++];
        } else {
            // 모든 전송이 완료
            txIndex = 1;
            isTransmitting = false;
            timer_ms = 0;  // 전송 완료 후 타이머 초기화
            timer_active = true;
            // SCI1.SCR.BIT.TIE = 0;  // Transmit Interrupt Disable (필요시)
            switchBuffers();  // 데이터 전송 완료 후 바로 버퍼 전환
        }
    }

}

void sciInit(void) {
    // Register Setting
    // RE = 1; Receiver enable
    // TE = 1; Transmitter enable
    // RIE = 1; Receive interrupt enable
    // TIE = 1; Transmit interrupt enable (enable later when needed)
    SCI1.SCR.BYTE = 0xF0;  // Example setup, adjust as necessary

    // SMR 설정
    // CM = 0; Asynchronous mode
    // CHR = 0; 8-bit data
    // PE = 0; Parity disable
    // PM = 0; No parity
    // STOP = 0; 1 stop bit
    SCI1.SMR.BYTE = 0x00;
    SCI1.BRR = 00;  // BuadRate 개발 환경에 따라 조절 필요.
}

int main(void) {

    initializeActiveBuffer();  // 버퍼 초기화 및 시작 마커 설정
    sciInit(); 
  
    // 메인로직 수행 시
    if(ready_to_tx) {
        readtToSend = true;
        // 100ms 시작을 대기
    }
}

/*
#include <stdbool.h>
#include <stdint.h>  // uint8_t와 uint32_t 타입을 위한 헤더

#define BUFFER_SIZE 1000

volatile int timer_ms = 0; // 1ms마다 증가하는 타이머 변수

uint8_t bufferA[BUFFER_SIZE];
uint8_t bufferB[BUFFER_SIZE];
int indexA = 0, indexB = 0;
uint8_t *activeBuffer = bufferA;
uint8_t *transmitBuffer = bufferB;
int *activeIndex = &indexA;
int *transmitIndex = &indexB;
bool isTransmitting = false;
bool readyToSend = false;
bool timer_active = true;

void switchBuffers(void) {
    if (!isTransmitting) {
        // 버퍼와 인덱스 포인터 교환
        uint8_t *tempBuffer = activeBuffer; // activeBuffer의 주소 값을 tempBuffer에 저장합니다.
        activeBuffer = transmitBuffer; // transmitBuffer의 주소 값을 activeBuffer에 저장합니다.
        transmitBuffer = tempBuffer; // tempBuffer의 주소 값을 transmitBuffer에 저장합니다.

        int *tempIndex = activeIndex; // activeIndex의 주소 값을 tempIndex에 저장합니다.
        activeIndex = transmitIndex; // transmitIndex의 주소 값을 activeIndex에 저장합니다.
        transmitIndex = tempIndex; // tempIndex의 주소 값을 transmitIndex에 저장합니다.

        // 로깅 버퍼의 데이터 길이를 종료 마커 위치에 추가
        int endIndex = 4 + *transmitIndex;  // 시작 마커의 크기가 4바이트
        uint32_t endMarker = 0xABCDEEEE;
        memcpy(transmitBuffer + endIndex, &endMarker, sizeof(endMarker));

        *transmitIndex = endIndex + 4; // 마커 크기 추가

        // activeBuffer 초기화 시 시작 마커 추가
        uint32_t startMarker = 0x0213E0E0;
        memcpy(activeBuffer, &startMarker, sizeof(startMarker));
        *activeIndex = 4; // 시작 마커의 크기만큼 인덱스 이동
    }
}

void logData(uint32_t data) {
    if (*activeIndex <= BUFFER_SIZE - 12) {
        activeBuffer[(*activeIndex)++] = (data >> 24) & 0xFF;
        activeBuffer[(*activeIndex)++] = (data >> 16) & 0xFF;
        activeBuffer[(*activeIndex)++] = (data >> 8 ) & 0xFF;
        activeBuffer[(*activeIndex)++] = (data >> 0 ) & 0xFF;
    }
}

void timer_interrupt_1ms(void) {
    if (timer_ms < 100 && timer_active) {
        timer_ms++; // 1ms마다 증가
    }

    if (timer_ms >= 100 && !isTransmitting && readyToSend) {
        uartSend(); // 데이터 전송 시작
        readyToSend = false;
        timer_active = false;  // 타이머 비활성화
    }
}

void uartSend(void) {
    if (*transmitIndex > 0 && !isTransmitting) {  // 전송할 데이터가 있는 경우에만 전송 시작
        isTransmitting = true;
        SCI1.TDR = transmitBuffer[0];  // 첫 바이트 전송
    }
}

// txi
void __attribute__((interrupt)) sci1_txi_isr(void) {
    if(specific_logic) {
        static int txIndex = 1;
        if (txIndex < *transmitIndex) {
            SCI1.TDR = transmitBuffer[txIndex++];
        } else {
            // 모든 전송이 완료
            txIndex = 1;
            isTransmitting = false;
            timer_ms = 0;  // 전송 완료 후 타이머 초기화
            timer_active = true;
            switchBuffers();  // 데이터 전송 완료 후 바로 버퍼 전환
        }
    }

}

void initializeBuffers(void) {
    // activeBuffer에 시작 마커 추가
    uint32_t startMarker = 0x0213E0E0;
    memcpy(activeBuffer, &startMarker, sizeof(startMarker));
    *activeIndex = 4;  // 시작 마커의 길이만큼 인덱스 이동
}

void main_logic(void) {

    initializeBuffers();  // 버퍼 초기화 및 시작 마커 설정
    
    if(ready_to_tx) {
        readtToSend = true;
        // 100ms 시작을 대기
    }
}
*/
