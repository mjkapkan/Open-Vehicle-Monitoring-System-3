/*
;    Project:       Open Vehicle Monitor System
;    Date:          14th September 2020
;
;    Changes:
;    1.0  Initial release
;
;    (C) 2011       Michael Stegen / Stegen Electronics
;    (C) 2011-2018  Mark Webb-Johnson
;    (C) 2011       Sonny Chen @ EPRO/DX
;    (C) 2020       Chris Staite
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
*/

#ifndef __RE_TOOLS_PID_H__
#define __RE_TOOLS_PID_H__

#include "can.h"

#include "freertos/task.h"
#include "freertos/queue.h"

#include <functional>

class OvmsReToolsPidScanner
{
  public:
    OvmsReToolsPidScanner(canbus* bus, uint16_t ecu, uint16_t start, uint16_t end);
    ~OvmsReToolsPidScanner();

    bool Complete() const { return m_currentPid == m_endPid; }
    uint16_t Ecu() const { return m_id; }
    uint16_t Start() const { return m_startPid; }
    uint16_t End() const { return m_endPid; }
    uint16_t Current() const { return m_currentPid; }

  private:
    void Ticker1(std::string event, void* data);

    void FrameCallback(const CAN_frame_t* frame, bool success);

    void IncomingPollFrame(const CAN_frame_t* frame);

    void SendNextFrame();

    static void Task(void *self);
    void Task();

    // A callback for when the frame has been sent
    std::function<void(const CAN_frame_t*, bool)> m_frameCallback;
    /// The CAN bus that is being used
    canbus* m_bus;
    /// The ID of the ECU to scan
    uint16_t m_id;
    /// The PID to start scanning from
    uint16_t m_startPid;
    /// The PID to stop scanning at
    uint16_t m_endPid;
    /// The current PID being scanned
    uint16_t m_currentPid;
    /// The current ticker value
    uint32_t m_ticker;
    /// When the last frame was sent
    uint32_t m_lastFrame;
    /// The number of bytes expected on a multi-frame response
    uint16_t m_mfRemain;
    /// The handle to the CAN task handler
    TaskHandle_t m_task;
    /// The handle to the CAN receive queue
    QueueHandle_t m_rxqueue;
};

#endif  // __RE_TOOLS_PID_H__