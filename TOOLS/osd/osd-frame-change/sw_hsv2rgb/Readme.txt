！！！下面为sw_hsv2rgb这个c文件的使用方法

1.执行hsv2rgb.c
    gcc hsv2rgb.c -o hsv2rgb 生成hsv2rgb可执行文件
2.执行./hsv2rgb dsthsv_h.dat dsthsv_s.dat dsthsv_v.dat
    生成-bgr_b  bgr_g  bgr_r三个文件
3.进入rgbxC文件执行gcc rgbxC.c -o rgbxC
    生成rgbxC可执行文件
4.执行./rgbxC  它会自动调用bgr_b、bgr_g、bgr_r三个文件
    生成rgb图片rgbx
5.只需要用7yuv把这个图片按照生成的ARGB的设置好图片格式和大小，打开就可以了
