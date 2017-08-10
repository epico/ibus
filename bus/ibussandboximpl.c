/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2017 Peng Wu <alexepico@gmail.com>
 * Copyright (C) 2017 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */


#include "ibussandboximpl.h"
#include "dbusimpl.h"
#include "ibusimpl.h"
#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <gio/gunixfdlist.h>
#include <gio/gunixinputstream.h>
#include <gio/gunixoutputstream.h>

struct _BusIBusSandBoxImpl {
    IBusService parent;

    /* instance members */
};

struct _BusIBusSandBoxImplClass {
    IBusServiceClass parent;

    /* class members */
};


/* functions prototype */
static void     bus_ibus_sandbox_impl_destroy   (BusIBusSandBoxImpl        *ibus);
static void     bus_ibus_sandbox_impl_service_method_call
                                        (IBusService        *service,
                                         GDBusConnection    *connection,
                                         const gchar        *sender,
                                         const gchar        *object_path,
                                         const gchar        *interface_name,
                                         const gchar        *method_name,
                                         GVariant           *parameters,
                                         GDBusMethodInvocation
                                                            *invocation);

/* The interfaces available in this class, which consists of a list of
 * methods this class implements and a list of signals this class may emit.
 * Method calls to the interface that are not defined in this XML will
 * be automatically rejected by the GDBus library (see src/ibusservice.c
 * for details. */
static const gchar sandbox_introspection_xml[] =
    "<node>\n"
    "  <interface name='org.freedesktop.Portals.IBus'>\n"
    "    <method name='Connect'>\n"
    "      <arg direction='in'  type='s' name='client_name' />\n"
    "    </method>\n"
    "  </interface>\n"
    "</node>\n";


G_DEFINE_TYPE (BusIBusSandBoxImpl, bus_ibus_sandbox_impl, IBUS_TYPE_SERVICE)

static void
bus_ibus_sandbox_impl_class_init (BusIBusSandBoxImplClass *class)
{
    IBUS_OBJECT_CLASS (class)->destroy =
            (IBusObjectDestroyFunc) bus_ibus_sandbox_impl_destroy;

    /* override the parent class's implementation. */
    IBUS_SERVICE_CLASS (class)->service_method_call =
            bus_ibus_sandbox_impl_service_method_call;
    /* register the xml so that bus_ibus_sandbox_impl_service_method_call will
     * be called on a method call defined in the xml
     * (e.g. 'Connect'.) */
    ibus_service_class_add_interfaces (IBUS_SERVICE_CLASS (class),
                                       sandbox_introspection_xml);
}

/**
 * bus_ibus_sandbox_impl_init:
 *
 * The constructor of BusIBusSandBoxImpl. Initialize all member variables of a BusIBusSandBoxImpl object.
 */
static void
bus_ibus_sandbox_impl_init (BusIBusSandBoxImpl *ibus)
{
}

/**
 * bus_ibus_sandbox_impl_destroy:
 *
 * The destructor of BusIBusSandBoxImpl.
 */
static void
bus_ibus_sandbox_impl_destroy (BusIBusSandBoxImpl *ibus_sandbox)
{
    IBUS_OBJECT_CLASS (bus_ibus_sandbox_impl_parent_class)->destroy (IBUS_OBJECT (ibus_sandbox));
}

/**
 * _ibus_sandbox_connect:
 *
 * Implement the "Connect" method call of the
 * org.freedesktop.sandbox.IBus interface.
 */
static void
_ibus_sandbox_connect (BusIBusSandBoxImpl    *ibus_sandbox,
                       GVariant              *parameters,
                       GDBusMethodInvocation *invocation)
{
    const gchar *client_name = NULL;  // e.g. "gtk-im"
    g_variant_get (parameters, "(&s)", &client_name);

    BusConnection *connection =
            bus_connection_lookup (g_dbus_method_invocation_get_connection (invocation));

    GUnixFDList *fds = g_unix_fd_list_new ();

    /* create a pair of sockets, and set to close-on-exec */
    GError *error = NULL;

    int fd_pair[2];
    /* failed on create socket pair. */
    if (socketpair (AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, 0, fd_pair)) {
        g_dbus_method_invocation_return_error (invocation,
                                               G_DBUS_ERROR,
                                               G_DBUS_ERROR_FAILED,
                                               "Connect failed!");
        g_object_unref (fds);
        return;
    }

    /* return the file descriptor to ibus client */
    int fd = fd_pair[0];
    g_unix_fd_list_append (fds, fd, &error);

    fd = fd_pair[1];
    GIOStream *stream = NULL;
    GDBusConnection *conn = NULL;
    stream = g_simple_io_stream_new (g_unix_input_stream_new (fd, TRUE),
                                     g_unix_output_stream_new (fd, TRUE));

    conn = g_dbus_connection_new_sync (stream,
                                       bus_server_get_guid (),
                                       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_SERVER |
                                       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS,
                                       NULL,
                                       NULL,
                                       &error);

    BusConnection *input_context_connection = bus_connection_new (conn);
    bus_dbus_impl_new_connection (bus_dbus_impl_get_default (),
                                  input_context_connection);

    if (g_object_is_floating (conn)) {
        /* bus_dbus_impl_new_connection couldn't handle the connection. */
        ibus_object_destroy ((IBusObject *)conn);
        g_object_unref (conn);

        g_dbus_method_invocation_return_error (invocation,
                                               G_DBUS_ERROR,
                                               G_DBUS_ERROR_FAILED,
                                               "Connect failed!");
        g_object_unref (fds);
        return;
    }

    g_dbus_method_invocation_return_value_with_unix_fd_list (invocation, NULL, fds);
    g_object_unref (fds);
    return;
}

/**
 * bus_ibus_sandbox_impl_service_method_call:
 *
 * Handle a D-Bus method call whose destination and interface name are
 * both "org.freedesktop.sandbox.IBus"
 */
static void
bus_ibus_sandbox_impl_service_method_call (IBusService           *service,
                                           GDBusConnection       *connection,
                                           const gchar           *sender,
                                           const gchar           *object_path,
                                           const gchar           *interface_name,
                                           const gchar           *method_name,
                                           GVariant              *parameters,
                                           GDBusMethodInvocation *invocation)
{
    if (g_strcmp0 (interface_name, IBUS_SANDBOX_INTERFACE_IBUS) != 0) {
        IBUS_SERVICE_CLASS (bus_ibus_sandbox_impl_parent_class)->service_method_call (
                        service, connection, sender, object_path,
                        interface_name, method_name,
                        parameters, invocation);
        return;
    }

    /* all methods in the xml definition above should be listed here. */
    static const struct {
        const gchar *method_name;
        void (* method_callback) (BusIBusSandBoxImpl *,
                                  GVariant *,
                                  GDBusMethodInvocation *);
    } methods [] =  {
        /* IBus sandbox interface */
        { "Connect",    _ibus_sandbox_connect },
    };

    gint i;
    for (i = 0; i < G_N_ELEMENTS (methods); i++) {
        if (g_strcmp0 (methods[i].method_name, method_name) == 0) {
            methods[i].method_callback ((BusIBusSandBoxImpl *) service,
                                        parameters,
                                        invocation);
            return;
        }
    }

    g_warning ("service_method_call received an unknown method: %s",
               method_name ? method_name : "(null)");
}

BusIBusSandBoxImpl *
bus_ibus_sandbox_impl_get_default (void)
{
    static BusIBusSandBoxImpl *ibus_sandbox = NULL;

    if (ibus_sandbox == NULL) {
        ibus_sandbox = (BusIBusSandBoxImpl *) g_object_new
            (BUS_TYPE_IBUS_SANDBOX_IMPL,
             "object-path", IBUS_SANDBOX_PATH_IBUS,
             NULL);
    }
    return ibus_sandbox;
}
