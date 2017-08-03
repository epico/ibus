/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
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
#ifndef __BUS_IBUS_SANDBOX_IMPL_H_
#define __BUS_IBUS_SANDBOX_IMPL_H_

#include <ibus.h>
#include "connection.h"
#include "component.h"
#include "inputcontext.h"
#include "factoryproxy.h"
#include "panelproxy.h"
#include "engineproxy.h"
#include "ibusimpl.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_IBUS_SANDBOX_IMPL              \
    (bus_ibus_sandbox_impl_get_type ())
#define BUS_IBUS_SANDBOX_IMPL(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_IBUS_SANDBOX_IMPL, BusIBusSandBoxImpl))
#define BUS_IBUS_SANDBOX_IMPL_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_IBUS_SANDBOX_IMPL, BusIBusSandBoxImplClass))
#define BUS_IS_IBUS_SANDBOX_IMPL(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_IBUS_SANDBOX_IMPL))
#define BUS_IS_IBUS_SANDBOX_IMPL_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_IBUS_SANDBOX_IMPL))
#define BUS_IBUS_SANDBOX_IMPL_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_IBUS_SANDBOX_IMPL, BusIBusSandBoxImplClass))

#define BUS_DEFAULT_IBUS_SANDBOX \
    (bus_ibus_sandbox_impl_get_default ())

G_BEGIN_DECLS

typedef struct _BusIBusSandBoxImpl BusIBusSandBoxImpl;
typedef struct _BusIBusSandBoxImplClass BusIBusSandBoxImplClass;

GType            bus_ibus_sandbox_impl_get_type             (void);

/**
 * bus_ibus_impl_get_default:
 * @returns: a BusIBusImpl object which is a singleton.
 *
 * Instantiate a BusIBusImpl object (if necessary) and return the object.
 */
BusIBusSandBoxImpl     *bus_ibus_sandbox_impl_get_default          (void);

G_END_DECLS
#endif
