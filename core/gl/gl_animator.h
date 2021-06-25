#pragma once

/**
 * @file gl_animator.h
 * @author JP Wang (wangjiaping@idea.edu.cn)
 * @brief 
 * @version 1.0
 * @date 2021-04-30
 * 
 * @copyright  
 * Cross-Platform Core Foundation (CPCF)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *      * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 *      * Neither the name of CPCF.  nor the names of its
 *        contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   
 */
/** \addtogroup Gl 
 * @ingroup CPCF
 *  @{
 */
#include "gl_basic.h"
#include "gl_math.h"

namespace gl
{
/** \addtogroup Gl_Animator
 * @ingroup Gl
 *  @{
 */
class aniArcball: public _details::_TransformationBase
{
	gl::ArcBallf	_arcball;
public:
	virtual void OnUserInputEvent(const os::UserInputEvent& x);
	virtual const rt::Mat4x4f& GetTransformation() const;
	virtual DWORD InterestedUserInputDevices() const { return os::UIDEV_MOUSE|os::UIDEV_VIEWPORT; }
};
/** @}*/
} // namespace gl
/** @}*/