#ifndef IMG_H
# define IMG_H

/*
** 課題スライドの「PPM連番を吐く描画API」を踏襲した最小構成。
** 背景(PPM)を読み込み、毎フレームその上に図形を描いて imgXXXX.ppm を出力する。
*/

# include <stddef.h>

# define WIDTH  1386
# define HEIGHT 851

typedef struct s_color
{
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
}	t_color;

typedef struct s_pt
{
	int	x;
	int	y;
}	t_pt;

/* 背景(PPM P6, 255固定)を読み込む。失敗したら0を返す */
int		img_load_background_ppm(const char *path);

/* 背景があるなら背景で初期化、なければ白で初期化 */
void	img_clear(void);

/* 連番で img0001.ppm, img0002.ppm... を出力 */
int		img_write(void);

/* 1ピクセル描画（範囲外は無視） */
void	img_putpixel(t_color c, int x, int y);

/* 塗りつぶし円（簡易） */
void	img_fillcircle(t_color c, double x, double y, double r);

/* 線分（Bresenham） */
void	img_line(t_color c, t_pt a, t_pt b);

/* ルート（点列）を太めに描く：線＋小円で視認性UP */
void	img_draw_route(t_color c, const t_pt *pts, size_t n, int thick);

void	img_filltriangle(t_color col, t_pt a, t_pt b, t_pt c);


#endif
