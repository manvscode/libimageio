/*
 * Copyright (C) 2010 Joseph A. Marrero.  http://www.manvscode.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _BLENDING_H_
#define _BLENDING_H_

#include <stdint.h>

#define channelblend_normal(a, b)       ((uint8_t)(a))
#define channelblend_lighten(a, b)      ((uint8_t)((b > a) ? b:a))
#define channelblend_darken(a, b)       ((uint8_t)((b > a) ? a:b))
#define channelblend_multiply(a, b)     ((uint8_t)((a * b) / 255))
#define channelblend_average(a, b)      ((uint8_t)((a + b) / 2))
#define channelblend_add(a, b)          ((uint8_t)(min(255, (a + b))))
#define channelblend_subtract(a, b)     ((uint8_t)((a + b < 255) ? 0:(a + b - 255)))
#define channelblend_difference(a, b)   ((uint8_t)(abs(a - b)))
#define channelblend_negation(a, b)     ((uint8_t)(255 - abs(255 - a - b)))
#define channelblend_screen(a, b)       ((uint8_t)(255 - (((255 - a) * (255 - b)) >> 8)))
#define channelblend_exclusion(a, b)    ((uint8_t)(a + b - 2 * a * b / 255))
#define channelblend_overlay(a, b)      ((uint8_t)((b < 128) ? (2 * a * b / 255):(255 - 2 * (255 - a) * (255 - b) / 255)))
#define channelblend_softlight(a, b)    ((uint8_t)((b < 128)?(2*((a>>1)+64))*((float)b/255):(255-(2*(255-((a>>1)+64))*(float)(255-b)/255))))
#define channelblend_hardlight(a, b)    (channelblend_overlay(b,a))
#define channelblend_colordodge(a, b)   ((uint8_t)((b == 255) ? b:min(255, ((a << 8 ) / (255 - b)))))
#define channelblend_colorburn(a, b)    ((uint8_t)((b == 0) ? b:max(0, (255 - ((255 - a) << 8 ) / b))))
#define channelblend_lineardodge(a, b)  (channelblend_add(a, b))
#define channelblend_linearburn(a, b)   (channelblend_subtract(a, b))
#define channelblend_linearlight(a, b)  ((uint8_t)(b < 128)?channelblend_linearburn(a,(2 * b)):channelblend_lineardodge(a,(2 * (b - 128))))
#define channelblend_vividlight(a, b)   ((uint8_t)(b < 128)?channelblend_colorburn(a,(2 * b)):channelblend_colordodge(a,(2 * (b - 128))))
#define channelblend_pinlight(a, b)     ((uint8_t)(b < 128)?channelblend_darken(a,(2 * b)):channelblend_lighten(a,(2 * (b - 128))))
#define channelblend_hardmix(a, b)      ((uint8_t)((channelblend_vividlight(a, b) < 128) ? 0:255))
#define channelblend_reflect(a, b)      ((uint8_t)((b == 255) ? b:min(255, (a * a / (255 - b)))))
#define channelblend_glow(a, b)         (channelblend_reflect(b,a))
#define channelblend_phoenix(a, b)      ((uint8_t)(min(a, b) - max(a, b) + 255))
#define channelblend_alpha(a, b, o)     ((uint8_t)(o * a + (1 - o) * b))
#define channelblend_alphaf(a, b, f, o) (channelblend_alpha(f(a, b), a, o))


#define colorblend_buffer(t, a, b, m)   (t)[0] = channelblend_##m((a)[0], (b)[0]),\
                                        (t)[1] = channelblend_##m((a)[1], (b)[1]),\
                                        (t)[2] = channelblend_##m((a)[2], (b)[2])

#define colorblend_buffer32(t, a, b, m)   (t)[0] = channelblend_##m((a)[0], (b)[0]),\
                                          (t)[1] = channelblend_##m((a)[1], (b)[1]),\
                                          (t)[2] = channelblend_##m((a)[2], (b)[2]),\
                                          (t)[3] = channelblend_##m((a)[3], (b)[3])


#define colorblend_normal(t,a,b)        (colorblend_buffer(t,a,b,normal))
#define colorblend_lighten(t,a,b)       (colorblend_buffer(t,a,b,lighten))
#define colorblend_darken(t,a,b)        (colorblend_buffer(t,a,b,darken))
#define colorblend_multiply(t,a,b)      (colorblend_buffer(t,a,b,multiply))
#define colorblend_average(t,a,b)       (colorblend_buffer(t,a,b,average))
#define colorblend_add(t,a,b)           (colorblend_buffer(t,a,b,add))
#define colorblend_subtract(t,a,b)      (colorblend_buffer(t,a,b,subtract))
#define colorblend_difference(t,a,b)    (colorblend_buffer(t,a,b,difference))
#define colorblend_negation(t,a,b)      (colorblend_buffer(t,a,b,negation))
#define colorblend_screen(t,a,b)        (colorblend_buffer(t,a,b,screen))
#define colorblend_exclusion(t,a,b)     (colorblend_buffer(t,a,b,exclusion))
#define colorblend_overlay(t,a,b)       (colorblend_buffer(t,a,b,overlay))
#define colorblend_softlight(t,a,b)     (colorblend_buffer(t,a,b,softlight))
#define colorblend_hardlight(t,a,b)     (colorblend_buffer(t,a,b,hardlight))
#define colorblend_colordodge(t,a,b)    (colorblend_buffer(t,a,b,colordodge))
#define colorblend_colorburn(t,a,b)     (colorblend_buffer(t,a,b,colorburn))
#define colorblend_lineardodge(t,a,b)   (colorblend_buffer(t,a,b,lineardodge))
#define colorblend_linearburn(t,a,b)    (colorblend_buffer(t,a,b,linearburn))
#define colorblend_linearlight(t,a,b)   (colorblend_buffer(t,a,b,linearlight))
#define colorblend_vividlight(t,a,b)    (colorblend_buffer(t,a,b,vividlight))
#define colorblend_pinlight(t,a,b)      (colorblend_buffer(t,a,b,pinlight))
#define colorblend_hardmix(t,a,b)       (colorblend_buffer(t,a,b,hardmix))
#define colorblend_reflect(t,a,b)       (colorblend_buffer(t,a,b,reflect))
#define colorblend_glow(t,a,b)          (colorblend_buffer(t,a,b,glow))
#define colorblend_phoenix(t,a,b)       (colorblend_buffer(t,a,b,phoenix))


#endif /* _BLENDING_H_ */
