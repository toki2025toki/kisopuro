#include "img.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
** buf: 出力フレーム
** bg : 背景（任意）
*/
static unsigned char	g_buf[HEIGHT][WIDTH][3];
static unsigned char	g_bg[HEIGHT][WIDTH][3];
static int				g_has_bg = 0;
static int				g_frame = 0;

/* ---------- PPM(P6)読み込み補助 ---------- */

static int	skip_ws_and_comments(FILE *fp)
{
	int	ch;

	while ((ch = fgetc(fp)) != EOF)
	{
		if (isspace(ch))
			continue;
		if (ch == '#')
		{
			while ((ch = fgetc(fp)) != EOF && ch != '\n')
				;
			continue;
		}
		ungetc(ch, fp);
		return (1);
	}
	return (0);
}

static int	read_int_token(FILE *fp, int *out)
{
	int	ch;
	int	val;

	if (!skip_ws_and_comments(fp))
		return (0);
	val = 0;
	while ((ch = fgetc(fp)) != EOF)
	{
		if (!isdigit(ch))
		{
			ungetc(ch, fp);
			break ;
		}
		val = val * 10 + (ch - '0');
	}
	*out = val;
	return (1);
}

int	img_load_background_ppm(const char *path)
{
	FILE	*fp;
	char	magic[3];
	int		w;
	int		h;
	int		maxval;
	size_t	want;

	fp = fopen(path, "rb");
	if (!fp)
		return (0);
	if (fread(magic, 1, 2, fp) != 2)
		return (fclose(fp), 0);
	magic[2] = '\0';
	if (strcmp(magic, "P6") != 0)
		return (fclose(fp), 0);
	if (!read_int_token(fp, &w) || !read_int_token(fp, &h)
		|| !read_int_token(fp, &maxval))
		return (fclose(fp), 0);
	if (w != WIDTH || h != HEIGHT || maxval != 255)
		return (fclose(fp), 0);

	/* ヘッダの後ろに1文字(改行)があることが多いので読み捨て */
	fgetc(fp);

	want = (size_t)WIDTH * (size_t)HEIGHT * 3u;
	if (fread(g_bg, 1, want, fp) != want)
		return (fclose(fp), 0);
	fclose(fp);
	g_has_bg = 1;
	return (1);
}

/* ---------- 描画API ---------- */

void	img_clear(void)
{
	int	y;
	int	x;

	if (g_has_bg)
	{
		memcpy(g_buf, g_bg, sizeof(g_buf));
		return ;
	}
	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			g_buf[y][x][0] = 255;
			g_buf[y][x][1] = 255;
			g_buf[y][x][2] = 255;
			x++;
		}
		y++;
	}
}

void	img_putpixel(t_color c, int x, int y)
{
	if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
		return ;
	g_buf[y][x][0] = c.r;
	g_buf[y][x][1] = c.g;
	g_buf[y][x][2] = c.b;
}

void	img_fillcircle(t_color c, double x, double y, double r)
{
	int	ix;
	int	iy;
	int	xmin;
	int	xmax;
	int	ymin;
	int	ymax;
	double	dx;
	double	dy;

	xmin = (int)(x - r - 1);
	xmax = (int)(x + r + 1);
	ymin = (int)(y - r - 1);
	ymax = (int)(y + r + 1);
	iy = ymin;
	while (iy <= ymax)
	{
		ix = xmin;
		while (ix <= xmax)
		{
			dx = (double)ix - x;
			dy = (double)iy - y;
			if (dx * dx + dy * dy <= r * r)
				img_putpixel(c, ix, iy);
			ix++;
		}
		iy++;
	}
}

/* ---------- 追加：塗りつぶし三角形（人の胴体用） ---------- */

/*
** 目的：
** - 3点(a,b,c)で作る三角形を塗りつぶして描く
** - "点が三角形の中か" を判定してピクセルを塗る（課題向けで分かりやすい）
*/
static int	sign_int(t_pt p1, t_pt p2, t_pt p3)
{
	return ((p1.x - p3.x) * (p2.y - p3.y)
		- (p2.x - p3.x) * (p1.y - p3.y));
}

static int	point_in_triangle(t_pt p, t_pt a, t_pt b, t_pt c)
{
	int	d1;
	int	d2;
	int	d3;
	int	has_neg;
	int	has_pos;

	d1 = sign_int(p, a, b);
	d2 = sign_int(p, b, c);
	d3 = sign_int(p, c, a);
	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
	return (!(has_neg && has_pos));
}

void	img_filltriangle(t_color col, t_pt a, t_pt b, t_pt c)
{
	int		minx;
	int		maxx;
	int		miny;
	int		maxy;
	int		x;
	int		y;
	t_pt	p;

	minx = a.x;
	if (b.x < minx) minx = b.x;
	if (c.x < minx) minx = c.x;
	maxx = a.x;
	if (b.x > maxx) maxx = b.x;
	if (c.x > maxx) maxx = c.x;

	miny = a.y;
	if (b.y < miny) miny = b.y;
	if (c.y < miny) miny = c.y;
	maxy = a.y;
	if (b.y > maxy) maxy = b.y;
	if (c.y > maxy) maxy = c.y;

	y = miny;
	while (y <= maxy)
	{
		x = minx;
		while (x <= maxx)
		{
			p.x = x;
			p.y = y;
			if (point_in_triangle(p, a, b, c))
				img_putpixel(col, x, y);
			x++;
		}
		y++;
	}
}


void	img_line(t_color c, t_pt a, t_pt b)
{
	int	dx;
	int	dy;
	int	sx;
	int	sy;
	int	err;
	int	e2;

	dx = abs(b.x - a.x);
	dy = abs(b.y - a.y);
	sx = (a.x < b.x) - (a.x > b.x);
	sy = (a.y < b.y) - (a.y > b.y);
	err = dx - dy;
	while (1)
	{
		img_putpixel(c, a.x, a.y);
		if (a.x == b.x && a.y == b.y)
			break ;
		e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			a.x += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			a.y += sy;
		}
	}
}

void	img_draw_route(t_color c, const t_pt *pts, size_t n, int thick)
{
	size_t	i;

	if (!pts || n == 0)
		return ;
	i = 1;
	while (i < n)
	{
		img_line(c, pts[i - 1], pts[i]);
		i++;
	}
	/* 太さを出したいので、点を小さな円で重ねる */
	i = 0;
	while (i < n)
	{
		img_fillcircle(c, (double)pts[i].x, (double)pts[i].y, (double)thick);
		i++;
	}
}

int	img_write(void)
{
	char	name[64];
	FILE	*fp;
	size_t	want;

	g_frame++;
	snprintf(name, sizeof(name), "img%04d.ppm", g_frame);
	fp = fopen(name, "wb");
	if (!fp)
		return (0);
	fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
	want = (size_t)WIDTH * (size_t)HEIGHT * 3u;
	if (fwrite(g_buf, 1, want, fp) != want)
		return (fclose(fp), 0);
	fclose(fp);
	return (1);
}
