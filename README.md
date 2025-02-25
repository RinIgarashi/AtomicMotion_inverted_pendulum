# クローラ型倒立振子

M5Stackの
[AtomS3](https://docs.m5stack.com/en/core/AtomS3)
と
[Atomic Motion Base](https://docs.m5stack.com/en/atom/Atomic%20Motion%20Base)
を使って製作したクローラ型のラジコンが倒立できるか実験してみた。

## 動作

### 倒立状態 (gif)

カーペットの上じゃないと立たない。
40~60秒くらいが限界。\
サーボの回転方向によって速度が異なるため、やや回転する。

<img src="images/demo.gif" width="50%">

### Teleplotによるデータの可視化

画面右上の ```Import layout``` から以下のjsonファイルを読み込むと同じ表示になる。\
[Teleplot テンプレート](teleplot\template.json)

<img src="images/teleplot.png" width="100%">

## システム構成

<img src="images/system.png" width="100%">

## 部品

### Rover

* [AtomS3](https://docs.m5stack.com/en/core/AtomS3)
(マイコン, IMU)
* [Atomic Motion Base](https://docs.m5stack.com/en/atom/Atomic%20Motion%20Base)
(モータードライバ, バッテリー)
* [FS90R](https://www.feetechrc.com/48v-13kg-analog-continuous-rudder-machine.html)
(連続回転サーボ)
* [連結式クローラー&スプロケットセット](https://www.tamiya.com/japan/products/70237/index.html)
(履帯)

<table>
  <tr>
    <td><img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/products/core/AtomS3/4.webp" width="100%"></td>
    <td><img src="https://static-cdn.m5stack.com/resource/docs/products/atom/Atomic%20Motion%20Base/img-40a0d2ba-04b3-4aa3-8417-0624762b3cc3.webp" width="100%"></td>
  </tr>
  <tr>
    <td><img src="https://www.feetechrc.com/Data/feetechrc/upload/image/20200612/4-5.png" width="100%"></td>
    <td><img src="https://d7z22c0gz59ng.cloudfront.net/japan_contents/img/usr/item/7/70237/70237_1.jpg" width="100%"></td>
  </tr>
</table>

### Monitor (additional)

* [M5Stack Core2](https://docs.m5stack.com/en/core/core2)
* [M5Stack Module HMI](https://docs.m5stack.com/en/module/HMI%20Module)

<table>
  <tr>
    <td><img src="https://static-cdn.m5stack.com/resource/docs/products/core/core2/core2_01.webp" width="100%"></td>
    <td><img src="https://static-cdn.m5stack.com/resource/docs/products/module/HMI%20Module/img-4c227abf-0a4c-4e3a-b711-b81a35899aaf.webp" width="100%"></td>
  </tr>
</table>
