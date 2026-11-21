# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# lnk2000 reads linker command files directly; there is no GNU-style -T flag
# translation layer.

set_property(TARGET linker PROPERTY link_script_option "")
