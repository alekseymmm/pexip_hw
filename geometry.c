#include <math.h>

#include "geometry.h"

int point_in_triangle(struct point *pt, struct point *v1, struct point *v2,
		      struct point *v3)
{
    float d1, d2, d3;
    int has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

int point_in_triangle2(struct point *pt, struct point *v1, struct point *v2,
		      struct point *v3)
{
	float area;
	float s, t;
	area = 0.5 *(-v2->y*v3->x + v1->y*(-v2->x + v3->x) + v1->x*(v2->y - v3->y) + v2->x*v3->y);
	s = 1/(2*area)*(v1->y*v3->x - v1->x*v3->y + (v3->y - v1->y)*pt->x + (v1->x - v3->x)*pt->y);
	t = 1/(2*area)*(v1->x*v2->y - v1->y*v2->x + (v1->y - v2->y)*pt->x + (v2->x - v1->x)*pt->y);

	return (s > 0 && t > 0 && (1-s-t > 0));
}
#if 0
bool intpoint_inside_trigon(intPoint s, intPoint a, intPoint b, intPoint c)
{
    int as_x = s.x-a.x;
    int as_y = s.y-a.y;

    bool s_ab = (b.x-a.x)*as_y-(b.y-a.y)*as_x > 0;

    if((c.x-a.x)*as_y-(c.y-a.y)*as_x > 0 == s_ab) return false;

    if((c.x-b.x)*(s.y-b.y)-(c.y-b.y)*(s.x-b.x) > 0 != s_ab) return false;

    return true;
}
#endif

void rotate_point(struct point *pt, int width, int height, struct point *res,
		   float a)
{
	int x_new, y_new;
	float x_dif, y_dif;
	int xc = width / 2;
	int yc = height / 2;

	float cosa = cos(a);
	float sina = sin(a);

	x_dif = pt->x - xc;
	//y_dif = pt->y - yc;
	y_dif = yc - pt->y;

	x_new = xc + (x_dif * cosa - y_dif * sina);
	y_new = yc - (x_dif * sina + y_dif * cosa);

	res->x = x_new;
	res->y = y_new;
}
