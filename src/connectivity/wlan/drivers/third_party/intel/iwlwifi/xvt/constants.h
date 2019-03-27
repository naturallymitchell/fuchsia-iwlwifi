/******************************************************************************
 *
 * Copyright(c) 2013 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2017   Intel Deutschland GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#ifndef __XVT_CONSTANTS_H
#define __XVT_CONSTANTS_H

#ifndef CPTCFG_IWLWIFI_SUPPORT_DEBUG_OVERRIDES
#define IWL_XVT_DEFAULT_DBGM_MEM_POWER	0
#define IWL_XVT_DEFAULT_DBGM_LMAC_MASK	0
#define IWL_XVT_DEFAULT_DBGM_PRPH_MASK	0

#else /* CPTCFG_IWLWIFI_SUPPORT_DEBUG_OVERRIDES */
#define IWL_XVT_DEFAULT_DBGM_MEM_POWER	(xvt->trans->dbg_cfg.XVT_DEFAULT_DBGM_MEM_POWER)
#define IWL_XVT_DEFAULT_DBGM_LMAC_MASK	(xvt->trans->dbg_cfg.XVT_DEFAULT_DBGM_LMAC_MASK)
#define IWL_XVT_DEFAULT_DBGM_PRPH_MASK	(xvt->trans->dbg_cfg.XVT_DEFAULT_DBGM_PRPH_MASK)

#endif /* CPTCFG_IWLWIFI_SUPPORT_DEBUG_OVERRIDES */

#endif /* __XVT_CONSTANTS_H */
