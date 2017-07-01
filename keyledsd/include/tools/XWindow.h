/** C++ wrapper for Xlib
 *
 * This simple wrapper presents a C++ interface for reading some information
 * about windows and devices from an X display.
 */
#ifndef TOOLS_WINDOW_H
#define TOOLS_WINDOW_H

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace std {
    template <> struct default_delete<::Display> {
        void operator()(::Display *p) const { XCloseDisplay(p); }
    };
}

namespace xlib {

class Display;

/****************************************************************************/

class Window final
{
public:
    typedef ::Window handle_type;
public:
                            Window(Display & display, handle_type window)
                             : m_display(display), m_window(window), m_classLoaded(false) {}

    Display &               display() const { return m_display; }
    handle_type             handle() const { return m_window; }

    void                    changeAttributes(unsigned long mask, const XSetWindowAttributes & attrs);

    std::string             name() const;
    std::string             iconName() const;
    const std::string &     className() const { if (!m_classLoaded) { loadClass(); } return m_className; }
    const std::string &     instanceName() const { if (!m_classLoaded) { loadClass(); } return m_instanceName; }

    std::string             getProperty(Atom atom, Atom type) const;

private:
    void                    loadClass() const;

private:
    Display &               m_display;
    handle_type             m_window;
    mutable std::string     m_className;
    mutable std::string     m_instanceName;
    mutable bool            m_classLoaded;
};

/****************************************************************************/

class Device final
{
public:
    typedef int             handle_type;
    enum { RawKeyPress, RawKeyRelease, RawButtonPress, RawButtonRelease } event_type;
public:
                            Device(Display & display, handle_type device)
                             : m_display(display), m_device(device) {}
                            Device(const Device &) = delete;
                            Device(Device && other) = default;

    static Device open(Display &, const std::string & devNode);

    Display &               display() const { return m_display; }
    handle_type             handle() const { return m_device; }

    std::string             getProperty(Atom atom, Atom type) const;

private:
    Display &               m_display;
    handle_type             m_device;
};

/****************************************************************************/

class Display final
{
public:
    typedef ::Display *     handle_type;
    typedef std::unordered_map<std::string, Atom> atom_map;
public:
                            Display(std::string name = std::string());
                            Display(const Display &) = delete;
                            Display(Display &&) = default;
    Display &               operator=(const Display &) = delete;
    Display &               operator=(Display &&) = delete;
                            ~Display();

    const std::string &     name() const { return m_name; }
    handle_type             handle() const { return m_display.get(); }
    Window &                root() { return m_root; }
    Atom                    atom(const std::string & name) const;

    int                     connection() const;

    std::unique_ptr<Window> getActiveWindow();

private:
    static std::unique_ptr<::Display> openDisplay(const std::string &);

private:
    std::unique_ptr<::Display> m_display;
    std::string             m_name;
    Window                  m_root;
    mutable atom_map        m_atomCache;
};

/****************************************************************************/

class Error : public std::runtime_error
{
public:
                            Error(const std::string & msg)
                             : std::runtime_error(msg) {}
                            Error(::Display *display, XErrorEvent *event)
                             : std::runtime_error(makeMessage(display, event)) {}
private:
    static std::string      makeMessage(::Display *display, XErrorEvent *event);
};

}

#endif