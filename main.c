#include "img.h"

#include <stdio.h>

/*
** 目的：
** - 背景地図（map.ppm）を読み込み
** - 調布駅→電通大、布田駅→電通大 の2ルートを
**   「ルートが伸びる＋人（〇+△）が進む」アニメとしてPPM連番出力
*/

/* 調布駅 -> 電気通信大学（赤） */
static const t_pt	g_route_chofu[] = {
	{481, 601},
	{502, 580},
	{485, 544},
	{491, 486},
	{508, 401},
	{525, 310},
	{459, 288}
};

/* 布田駅 -> 電気通信大学（青） */
static const t_pt	g_route_fuda[] = {
	{1010, 791},
	{1019, 676},
	{1051, 480},
	{910, 442},
	{755, 388},
	{557, 318},
	{459, 288}
};


t_color green = {60, 220, 120};


/*
** 人型（〇+△）を描く
** - (x,y) は胴体中心付近
*/
static void	draw_person(double x, double y, t_color head, t_color body)
{
	t_pt	top;
	t_pt	left;
	t_pt	right;

	/* 頭（〇） */
	img_fillcircle(head, x, y - 18.0, 10.0);

	/* 胴体（△） */
	top.x = (int)x;
	top.y = (int)(y - 6);
	left.x = (int)(x - 14);
	left.y = (int)(y + 20);
	right.x = (int)(x + 14);
	right.y = (int)(y + 20);
	img_filltriangle(body, top, left, right);
}

/*
** 進捗t(0.0-1.0)に応じて
** - ルートを伸ばす（0..i まで）
** - 人の位置を点列上で線形補間して描く
*/
static void	draw_progress(
	t_color route_c,
	t_color head_c,
	t_color body_c,
	const t_pt *pts,
	size_t n,
	double t
)
{
	size_t	i;
	double	pos;
	double	k;
	t_pt	a;
	t_pt	b;
	double	x;
	double	y;

	if (!pts || n < 2)
		return ;
	if (t < 0.0)
		t = 0.0;
	if (t > 1.0)
		t = 1.0;

	/* ルートが伸びる演出 */
	i = (size_t)((double)(n - 1) * t);
	img_draw_route(route_c, pts, i + 1, 10);

	/* 人の位置（線形補間） */
	pos = t * (double)(n - 1);
	i = (size_t)pos;
	if (i >= n - 1)
		i = n - 2;
	k = pos - (double)i;

	a = pts[i];
	b = pts[i + 1];
	x = (double)a.x + ((double)b.x - (double)a.x) * k;
	y = (double)a.y + ((double)b.y - (double)a.y) * k;

	draw_person(x, y, head_c, body_c);
}

/* 駅/目的地マーカー（固定） */
static void	draw_landmarks(void)
{
	t_color	c_station;
	t_color	c_edge;
	t_color	c_goal;

	c_station.r = 255;
	c_station.g = 255;
	c_station.b = 255;

	c_edge.r = 0;
	c_edge.g = 0;
	c_edge.b = 0;

	c_goal.r = 255;
	c_goal.g = 255;
	c_goal.b = 0;

	/* 調布駅 */
	img_fillcircle(c_edge, 481, 601, 14.0);
	img_fillcircle(c_station, 481, 601, 12.0);

	/* 布田駅 */
	img_fillcircle(c_edge, 1010, 791, 14.0);
	img_fillcircle(c_station, 1010, 791, 12.0);

	/* 電通大（ゴール） */
	img_fillcircle(c_edge, 459, 288, 18.0);
	img_fillcircle(c_goal, 459, 288, 16.0);
}

int	main(void)
{
	const int	frames_each = 10;
	int			i;
	double		t;
	size_t		nc;
	size_t		nf;

	t_color		red;
	t_color		blue;
	t_color		route_gray;
	t_color		head_white;
	t_color		goal_green;

	nc = sizeof(g_route_chofu) / sizeof(g_route_chofu[0]);
	nf = sizeof(g_route_fuda) / sizeof(g_route_fuda[0]);

	red = (t_color){255, 60, 60};
	blue = (t_color){60, 120, 255};
	route_gray = (t_color){160, 160, 160}; /* 道路の上で潰れにくい薄グレー */
	head_white = (t_color){255, 255, 255};
	goal_green = (t_color){60, 220, 120};

	/* 背景地図の読み込み */
	if (!img_load_background_ppm("map.ppm"))
	{
		printf("ERROR: map.ppm を読み込めません。\n");
		printf("先に: make ppm  （または convert map.png map.ppm）\n");
		return (1);
	}

	/* Scene1: 調布→電通大（赤） */
	i = 0;
	while (i < frames_each)
	{
		t = (double)i / (double)(frames_each - 1);
		img_clear();
		draw_landmarks();
		draw_progress(green, red, red, g_route_chofu, nc, t);
		if (!img_write())
			return (1);
		i++;
	}

	/* Scene2: 布田→電通大（青） */
	i = 0;
	while (i < frames_each)
	{
		t = (double)i / (double)(frames_each - 1);
		img_clear();
		draw_landmarks();
		draw_progress(green, blue, blue, g_route_fuda, nf, t);
		if (!img_write())
			return (1);
		i++;
	}

	/* Scene3: 両方同時（比較） */
	i = 0;
	while (i < frames_each)
	{
		t = (double)i / (double)(frames_each - 1);
		img_clear();
		draw_landmarks();

		draw_progress(route_gray, head_white, red, g_route_chofu, nc, t);
		draw_progress(route_gray, head_white, blue, g_route_fuda, nf, t);

		/* ゴール強調（点滅風） */
		if (i % 2 == 0)
			img_fillcircle(goal_green, 459, 288, 22.0);

		if (!img_write())
			return (1);
		i++;
	}
	return (0);
}
