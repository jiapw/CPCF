#pragma once
/** \addtogroup gl 
 * @ingroup CPCF
 *  @{
 */
/**
 * @file gl_math.h
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
#include "../rt/runtime_base.h"
#include "../rt/small_math.h"


namespace gl
{

template <typename t_Val>
class ArcBall
{
    rt::Vec<t_Val,3>		sphereStart, sphereEnd;
    rt::Quaternion<t_Val>	cubeQuat, cubeQuatStart;
    rt::Quaternion<t_Val>	mouseQuat;
protected:
    int m_lx, m_ly, m_ux, m_uy;

    rt::Vec<t_Val,2> m_Center;
	rt::Mat4x4<t_Val> m_Rotation;

    INLFUNC void transCoord(const rt::Vec2i &p, rt::Vec<t_Val,2> &res)
    {
        res.x = (t_Val)((p.x - m_Center.x) / (0.5 * (m_ux - m_lx)));
        res.y = -(t_Val)((p.y - m_Center.y) / (0.5 * (m_uy - m_ly)));
    }

    INLFUNC void getSpherePoint(const rt::Vec<t_Val,2> &p, rt::Vec<t_Val,3> &res)
    {
        t_Val r = p.x * p.x + p.y * p.y;
        res.x = p.x;
        res.y = p.y;

        if ( r - (t_Val)1.0 > -rt::TypeTraits<t_Val>::Epsilon() )
        {
            t_Val sr = (t_Val)sqrt(r);
            res.x /= sr;
            res.y /= sr;
            res.z = (t_Val)0.0;
        }
        else
            res.z = (t_Val)sqrt(1.0 - r);
    }

    INLFUNC void getQuat(const rt::Vec<t_Val,3> &p, const rt::Vec<t_Val,3> &q, rt::Quaternion<t_Val> &res)
    {
        res.Quat_Scal() = (t_Val)p.Dot(q);
        res.Quat_Vec().CrossProduct(p, q);
    }

public:

	INLFUNC const rt::Quaternion<t_Val>& operator = (const rt::Quaternion<t_Val>& x)
	{	cubeQuat = x;
		cubeQuat.ExportRotationMatrix(m_Rotation);
		return x;
	}

	INLFUNC operator const rt::Quaternion<t_Val> () const { return cubeQuat; }
	INLFUNC operator rt::Quaternion<t_Val> (){ return cubeQuat; }

    INLFUNC ArcBall()
    {   m_lx = m_ly = m_ux = m_uy = 0;
		rt::Zero(m_Center);
        Initialize();
    }

    INLFUNC void SetViewport(int lx, int ly, int ux, int uy)
    {
        ASSERT( lx <= ux && ly <= uy );

        m_lx = lx;
        m_ly = ly;
        m_ux = ( lx == ux ? ux + 1 : ux );
        m_uy = ( ly == uy ? uy + 1 : uy );

        m_Center.x = (t_Val)(0.5 * (lx + ux));
        m_Center.y = (t_Val)(0.5 * (ly + uy));
    }

    INLFUNC void Initialize()
    {
        mouseQuat.SetIdentity();
        cubeQuat.SetIdentity();
        cubeQuatStart.SetIdentity();
		rt::Zero(sphereStart);
		rt::Zero(sphereEnd);
		rt::Zero(m_Rotation);
        m_Rotation(3, 3) = (t_Val)1.0;
        cubeQuat.ExportRotationMatrix(m_Rotation);
    }

    INLFUNC void OnDragBegin(const rt::Vec2i &p)
    {
        rt::Vec<t_Val,2> q;
        transCoord(p, q);
        getSpherePoint(q, sphereStart);
        cubeQuatStart = cubeQuat;
        mouseQuat.SetIdentity();
    }
    INLFUNC void OnDrag(const rt::Vec2i &p)
    {	rt::Vec<t_Val,2> q;
        transCoord(p, q);
        getSpherePoint(q, sphereEnd);
        getQuat(sphereStart, sphereEnd, mouseQuat);

        cubeQuat = mouseQuat;
        cubeQuat.Multiply(cubeQuatStart);
        cubeQuat.Normalize();
        cubeQuat.ExportRotationMatrix(m_Rotation);
    }
	INLFUNC void OnDragBegin(int x, int y){ OnDragBegin(rt::Vec2i(x, y)); }
	INLFUNC void OnDrag(int x, int y){ OnDrag(rt::Vec2i(x, y)); }

    template <class t_Mat>
	void GetMatrix3x3(t_Mat &m){ m = m_Rotation; }

    template <class t_Mat>
    void GetMatrix4x4(t_Mat &m){ m = m_Rotation; }

    template <class t_Mat>
    void GetInverseMatrix3x3(t_Mat &m)
    {
        m(0, 0) = m_Rotation(0, 0);
        m(0, 1) = m_Rotation(1, 0);
        m(0, 2) = m_Rotation(2, 0);
        m(1, 0) = m_Rotation(0, 1);
        m(1, 1) = m_Rotation(1, 1);
        m(1, 2) = m_Rotation(2, 1);
        m(2, 0) = m_Rotation(0, 2);
        m(2, 1) = m_Rotation(1, 2);
        m(2, 2) = m_Rotation(2, 2);
    }

    template <class t_Mat>
    void GetInverseMatrix4x4(t_Mat &m)
    {
        m(0, 0) = m_Rotation(0, 0);
        m(0, 1) = m_Rotation(1, 0); 
        m(0, 2) = m_Rotation(2, 0);
        m(0, 3) = m_Rotation(3, 0);
        m(1, 0) = m_Rotation(0, 1);
        m(1, 1) = m_Rotation(1, 1);
        m(1, 2) = m_Rotation(2, 1);
        m(1, 3) = m_Rotation(3, 1);
        m(2, 0) = m_Rotation(0, 2);
        m(2, 1) = m_Rotation(1, 2);
        m(2, 2) = m_Rotation(2, 2);
        m(2, 3) = m_Rotation(3, 2);
        m(3, 0) = m_Rotation(0, 3);
        m(3, 1) = m_Rotation(1, 3);
        m(3, 2) = m_Rotation(2, 3);
        m(3, 3) = m_Rotation(3, 3);
    }

    template <class t_Mat>
    void LoadMatrix(t_Mat &m)
    {
       Initialize();
       cubeQuat.ImportRotationMatrix(m);

       memset(m_Rotation.m, 0, sizeof(m_Rotation.m));
       m_Rotation.m33 = (t_Val)1.0;
       m_Rotation(0, 0) = m(0, 0);
       m_Rotation(0, 1) = m(0, 1);
       m_Rotation(0, 2) = m(0, 2);
       m_Rotation(1, 0) = m(1, 0);
       m_Rotation(1, 1) = m(1, 1);
       m_Rotation(1, 2) = m(1, 2);
       m_Rotation(2, 0) = m(2, 0);
       m_Rotation(2, 1) = m(2, 1);
       m_Rotation(2, 2) = m(2, 2);
    }

	const rt::Mat4x4<t_Val>& GetMatrix() const{ return m_Rotation; }
};

typedef ArcBall<float>	ArcBallf;
typedef ArcBall<double> ArcBalld;

} // namespace gl
/** @}*/    