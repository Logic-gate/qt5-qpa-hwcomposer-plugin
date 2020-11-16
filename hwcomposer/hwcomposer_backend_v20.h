/****************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
** Contact: Thomas Perl <thomas.perl@jolla.com>
**
** This file is part of the hwcomposer plugin.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef HWCOMPOSER_BACKEND_V20_H
#define HWCOMPOSER_BACKEND_V20_H

#ifdef HWC_PLUGIN_HAVE_HWCOMPOSER1_API

#include "hwcomposer_backend.h"
// libhybris access to the native hwcomposer window
#include <hwcomposer_window.h>

#include <hybris/hwc2/hwc2_compatibility_layer.h>

#include <QBasicTimer>

#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>


class HwcProcs_v20;
class QWindow;

struct free_delete
{
    void operator()(void* x) { free(x); }
};


  enum class DisplayName
{
    primary = HWC_DISPLAY_PRIMARY,
    external = HWC_DISPLAY_EXTERNAL,
#ifdef ANDROID_CAF
    tertiary = HWC_DISPLAY_TERTIARY,
#endif
    virt = HWC_DISPLAY_VIRTUAL
};



struct DisplayContents
{
    DisplayName name;
};



typedef std::unique_ptr<hwc2_compat_display_t, free_delete> hwc2_compat_display_ptr;
typedef std::unique_ptr<HWC2DisplayConfig, free_delete> HWC2DisplayConfig_ptr;

inline auto as_hwc_display(DisplayName name) -> int
{
    return static_cast<int>(name);
}

class HwComposerBackend_v20 : public QObject, public HwComposerBackend {
public:
    HwComposerBackend_v20(hw_module_t *hwc_module, void *libminisf, DisplayName display_name);
    virtual ~HwComposerBackend_v20();

    virtual EGLNativeDisplayType display();
    virtual EGLNativeWindowType createWindow(int width, int height);
    virtual void destroyWindow(EGLNativeWindowType window);
    virtual void swap(EGLNativeDisplayType display, EGLSurface surface);
    virtual void sleepDisplay(bool sleep);
    virtual float refreshRate();
    virtual bool getScreenSizes(int *width, int *height, float *physical_width, float *physical_height);

    virtual bool requestUpdate(QEglFSWindow *window) Q_DECL_OVERRIDE;

    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;
    void handleVSyncEvent();
    bool event(QEvent *e) Q_DECL_OVERRIDE;

    void onHotplugReceived(int32_t sequenceId, hwc2_display_t display,
                           bool connected, bool primaryDisplay);
   //virtual void hdmi_test(DisplayName display_name, bool primaryDisplay, hwc2_display_t display);

    static int composerSequenceId;

    void invalidate() noexcept;
   


private:

    hwc2_compat_device_t* hwc2_device;
    hwc2_compat_display_t* hwc2_primary_display;
    hwc2_compat_layer_t* hwc2_primary_layer;

    struct Callbacks
    {
        std::function<void()> handleVSyncEvent;
        std::function<void(int32_t, DisplayName, bool)> onHotplugReceived;
        std::function<float()> refreshRate();

    };

    std::mutex callback_map_lock;
    std::unordered_map<int, hwc2_compat_display_ptr> hwc2_displays;
    std::atomic<bool> is_plugged[HWC_NUM_DISPLAY_TYPES];
    std::unordered_map<void const*, Callbacks> callback_map;
    std::unordered_map<int, bool> active_displays;
    std::unordered_map<int, int> lastPresentFence;
    std::unordered_map<int, std::vector<hwc2_compat_layer_t*>> display_contents;
    
    bool m_displayOff;
    QBasicTimer m_deliverUpdateTimeout;
    QBasicTimer m_vsyncTimeout;
    QSet<QWindow *> m_pendingUpdate;
    HwcProcs_v20 *procs;

   
};


#endif /* HWC_PLUGIN_HAVE_HWCOMPOSER1_API */

#endif /* HWCOMPOSER_BACKEND_V11_H */
