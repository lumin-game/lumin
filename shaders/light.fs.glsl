#version 330
// From vertex shader
in vec3 vcolor;
in vec2 vpos;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float lightRadius;

uniform vec4 collisionEqs[256]; // Equations for edges that will block the light
uniform int collisionEqCount;	// Actual count, because openGL doesn't support dynamic uniform arrays
// I am assuming we don't have more than 256 such edges

// Output color
layout(location = 0) out vec4 color;

void main()
{
	float epsilon = 0.001f;
	color = vec4(fcolor * vcolor, 1.0);
	
	// Look for collisions per pixel
	bool collisionFound = false;

	// Given
	// line1 : x1 = a1 + b1*t1, y1 = c1 + d1*t1
	// line2 : x2 = a2 + b2*t2, y2 = c2 + d2*t2
	// A collision means a pair of t1 and t2 that are both 0 < t < 1

	// line1 here will be our ray trace from our origin to current pixel position (vpos)
	// as such, a1 and c1 is always 0.
	// line2 is one of the collision equations passed in

	// With line1 simplification, algebra gives:
	// t2 = (d1*a2/b1 - c2) / (d2 - d1*b2/b1)
	// t1 = (a2 + b2 * t2) / b1

	// However we must consider the case where b1 == 0
	// In that case we use the alternate equations
	// t2 = -a2/b2
	// t1 = (c2 + d2*t2) / d1

	if (-epsilon < vpos.x && vpos.x < epsilon) // when b1 == 0
	{
		for (int i = 0; i < collisionEqCount; ++i)
		{
			vec4 collisionEq = collisionEqs[i];
			float t2 = (-collisionEq.x) / collisionEq.y;
			float t1 = (collisionEq.z + (collisionEq.w * t2)) / vpos.y;

			if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1)
			{
				discard;
			}
		}
	}
	else 
	{
		for (int i = 0; i < collisionEqCount; ++i)
		{
			vec4 collisionEq = collisionEqs[i];
			float t2 = ((vpos.y * collisionEq.x / vpos.x) - collisionEq.z) / (collisionEq.w - (vpos.y * collisionEq.y / vpos.x));
			float t1 = (collisionEq.x + (collisionEq.y * t2)) / vpos.x;

			if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1)
			{
				discard;
			}
		}
	}

	//TODO: Prolly don't hardcode these values
	float radius = distance(vec2(0.0), vpos);
	if (radius > lightRadius)
	{
		discard;
	}

	color.xyz = vec3(1.0, 1.0, 1.0);
	color.a = max(0.00, min(0.8, (1 - radius / lightRadius)));	
}
