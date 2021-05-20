#include "../../core/rt/small_math.h"
#pragma warning(disable: 4838)

#include "test.h"
void rt::UnitTests::smallmath()
{
	{
		rt::Vec4d	c;
		c.x = 1e-100;
		c.y = 1e100;
		c.z = 100.4;
		c.w = 0;
		_LOG(c);
	}

	{	rt::Vec3f	c;
	c.x = 1e-10f;
	c.y = 1e10f;
	c.z = 100.4f;
	_LOG(c);
	}

	rt::Vec3d	a({ 1.2, 3.4, 5.6 }), b({ -6.5, -4.3, 2.1 });
	rt::String s = rt::SS("STR-EXP: a=(") + a + ')';
	_LOG(s);
	a.Normalize();
	_LOG(a);
	_LOG(a << " dot " << b << " = " << a.Dot(b));

	rt::Randomizer r(7749);
	rt::Mat3x3d	m;
	for (double& x : m._p)
		x = r.GetNext() * 123. / INT_MAX - 88.;
	_LOG(m);
	m.Transpose();
	_LOG(m);

	rt::Quaterniond  quat;
	quat.ImportRotationMatrix(m, a);
	quat.ExportRotationMatrix(m);
	_LOG(m);
	_LOG("scale = " << a);

	{
		rt::Vec3f a(1, 2, 3), b(-1, -1, -1);
		_LOG((a + b * 2) + 2);
		//_LOG(a*b + a*b);
	}
}