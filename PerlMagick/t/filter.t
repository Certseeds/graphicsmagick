#!/usr/local/bin/perl
#
# Test image filter.
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1, print "1..43\n"; }
END {print "not ok 1\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';

testFilter('input.miff', 'Blur',
  q/0.0x1.0/,
  '5f9da8b86a8751391d9d33c0f422bb6b7fd52e517f3f215eb6776ad1987c5c97',
  '9a4e6e77fd7aa20d99a2d086b15627432bd78750cdf5d336ee45c6c6ea2745c3');

++$test;
testFilter('input.miff', 'Border',
  q/10/,
  '64d5519395a5facdba262efa93fc7243d0b384433f45c80199bb98e781b417fe');

++$test;
testFilter('input.miff', 'Channel',
  q/Red/,
  'cfdb638c95a4ab24dbf93cad1959bd94df6465611f03bc8ca8bc562d353655d2');

++$test;
testFilter('input.miff', 'Chop',
  q/10x10!/,
  'f30f652c3dad0f84474ec6fef65b34abef704bf5d4d1332a1fd79164b29d5a8b');

++$test;
testFilter('input.miff', 'ColorFloodfill',
  q/+25+45/,
  '4f575497a2a7bd00c078d169f7e21309b9dab1319c9601c900f233ca4d3996a6');

++$test;
testFilter('input.miff', 'Colorize',
  q/red/,
  'cd24b808aa19a0710563cf4705ebf8efc3cddd82a7d3648c9121c5058f4c9cf5');

++$test;
testFilter('input.miff', 'Contrast',
  q/1/,
  'b3ddcb8d22d1dfb9ffbb288f07fdf64a90c415787fd2b71e5f5c81d63415f490',
  '46a67f879c6e1849fdcabf14f1c48e19945705411e63db27e5a6fb6d60909bb1');

++$test;
testFilter('input.miff', 'Convolve',
  [1, 2, 1, 2, 4, 2, 1, 2, 1],
  '94927a3799d1f2938943b1bb12b7079b7485b0f912c608ea26e5b551bf5a5264',
  '0e68bb6a5ffdfc678fa2008520c3d93204c33cb5e070c35ea7ae579d44f56a80');

++$test;
testFilter('input.miff', 'Crop',
  q/10x10!/,
  '886abd0600151937beea6143c6c8ab149624ceddc40076cb28072a4c5c904bcf');

++$test;
testFilter('input.miff', 'Despeckle',
  q/ /,
  '8c63f4f807d48648c73f2eee9c278f068deae3093b4d917d91eff2f5e9ed8551',
  'a1496ec3554953265568f18d175d30cd227ca53b8337149daf6b4ae196418e81');

++$test;
testFilter('input.miff', 'Edge',
  q/3/,
  '077ae4e1fa8db5e76a60a5764faf3b50fb15a9db0a3eb6d93f50bdbb3455f361',
  'd74fa0ccfbb0e2e345b911dce248d1821027db1afffa3b3fc7b140a69d29b2fb');

++$test;
testFilter('input.miff', 'Emboss',
  q/3/,
  '8b322f279343c309ab2b6875ff5a45f4f7f0b18f958782ea89799bd26c72db90',
  '0f989782ed2b9c6eeb1889e41c76e72be4330edfbf04536639566fdcb5f48fae');

++$test;
testFilter('input.miff', 'Equalize',
  q/ /,
  'eef347707dd430c87fc90f63445fe4bf4e59e64d60d51db43cb9aae57f960e48',
  '17e317a255885296bbd7bcd6db79d99605826c60e4df8e51fd9930c28b072e02');

++$test;
testFilter('input.miff', 'Flip',
  q/ /,
  '33d7e8ab8f3e51619191bac5b0234c126a0fca006d39a682b1ddb6d1e515b8eb');

++$test;
testFilter('input.miff', 'Flop',
  q/ /,
  'bba4072d75dd5696176c8b7862724fe0309f60b3b67cc25583077ff5dcac1c9a');

++$test;
testFilter('input.miff', 'Frame',
  q/10x10/,
  '808ad51705e83a0bbc21720b770f198949df0f99a7f060a93eba22ada7768240', 
  '90a0ef26496810bef40cef8e5d3bc28cb64676e2b8b6c43268257e7c481b7452');

++$test;
testFilter('input.miff', 'Gamma',
  q/2.2/,
  '8f79b178b0c64f6b063a10ad01c7b4828307765b8ce24d6d100a22bd7d91da60',
  '18e4c9ebb351a562c5eb844cdfbf091933c34bd911a25841b19c8e365828e43e');

++$test;
testFilter('input.miff', 'Implode',
  q/0.5/,
  '587985c66eec744fa48e8b6acafaa2bb5c789472134fb89355351179bfb10997',
  '800bd416e3b963aa43bdff57bd511289800eda82a35f172152ee9cff1b45fe32');

++$test;
testFilter('input.miff', 'Magnify',
  q/ /,
  'e5e4dbaaea4faba87326d1ccde1f84aaa60c41518bd242f4d65dd9f37d2f2dc5',
  '31c493286e8bd7ba5a8f36ba36edb05e1f8a58442347fd0021fb9c3a6a692822');

++$test;
testFilter('input.miff', 'MatteFloodfill',
  q/+25+45/,
  '31c493286e8bd7ba5a8f36ba36edb05e1f8a58442347fd0021fb9c3a6a692822',
  '100a892e7d683a23a73ea67fef7efc7b57d48efc1e1bba4cb7117e6ce371a3fe');

++$test;
testFilter('input.miff', 'Minify',
  q/ /,
  '0b349f8911838b4030d23accd694312d50b3c3ca1d859e4e54a33ffc210fecd2',
  '5286bc18c5c2dd0d2970be7fe4b31ff10b0612ea137129c3e1d92602b2c21f4b');

++$test;
testFilter('input.miff', 'Modulate',
  q/60/,
  '6e46f9e5399b8265a1c522750dbdd227d1dcae084d18f00a79b27b5638670bc5',
  '7ff720b84e67ab83b084ca394dbab311ef8954c6404e5709aa2055d46bfc3aac');

++$test;
testFilter('input.miff', 'Negate',
  q/False/,
  '70eb168ab1fb62363a2353fca5b3565b162ad26e383bf3632993e0ee595b34b8');

++$test;
testFilter('input.miff', 'Normalize',
  q/ /,
  'eec8915ee71f18ce398da7254517d982e9adcd5d1c23d5ce014ce9a30c57f855',
  '15b6d228cd32a5004f1947f4ce319d537c752f823298fe094717a90fb34479ca');

++$test;
testFilter('input.miff', 'OilPaint',
  q/3/,
  'c2cb7a8a7d8a2c501aed113b38ea886cf718401cd686015e15dbc782a43d3611',
  'b0dda0f41c5abf156cdc4c1110205ce17efc04cbed36d74eb43d8a1c200c173f');

++$test;
testFilter('input.miff', 'Opaque',
  q/#fefefd/,
  'a5d122b2b7a0944d835243afd4a81815bbea5bc1ae6c3a6ba79301662b4474a1');

++$test;
testFilter('input.miff', 'Quantize',
  q/128/,
  'a690f2732ba25e3a101ba3d166b5c9345198606d73eae0f7c8b7fc8e61bbba01',
  'dc224abeddcb1bb0efdaad191167935b99daaa989cf57dc6fe1aa11b86a93ce3');

++$test;
testFilter('input.miff', 'Raise',
  q/10x10/,
  '58fb8633fe6b4a9e97957aabbc3bf5dbbd2c035306e5e1dd2d93f2e18476ee7c',
  'ef9158abb43a7955f05b826ac8ce2f8e643a76df1ed829a9ea9c290e7d4ad54a');

++$test;
testFilter('input.miff', 'Resize',
  q/50%/,
  'e37ea53b8e167838add16e4a46be3689150df6b8d496da15ec41bee74d2fb9f1',
  'f401a1f40401518dce6a77ef250c8b63cab0b22087176aa7570cbbf230b3c82d');

++$test;
testFilter('input.miff', 'Roll',
  q/+10+10/,
  'c492f7a1938e6ee5676c3eead3eb1754d8c5b59bffa132efda0dd49f436a210a');

++$test;
testFilter('input.miff', 'Rotate',
  q/10/,
  '5bf1ebdbad22c4e650e2e49602790157331ae8080c6cdade9d67363ef12b28c0',
  '1641865f619d3b37b60967233d5841554ba79a9e58622a88bc5fb0c04961f3f1');

++$test;
testFilter('input.miff', 'Sample',
  q/50%/,
  '549d7097906f38acb7f07af2a7e4721f72c0dc1b40b1981c4ba95355ef7fde0b');

++$test;
testFilter('input.miff', 'Scale',
  q/50%/,
  '2bee86bfedf751c4a58d791427d4c24fac99de06808588c22e3f54a358ccac3c',
  '7720d2340dfa14a2dbff6c2ad491c2ffd3366bebfd6918eb3b25035518236ca9');

++$test;
testFilter('input.miff', 'Segment',
  q/1/,
  'fa219869b3cc8560def29c2f28974d75d3eede767c70d784be931fe44c6b1172');

++$test;
testFilter('input.miff', 'Shade',
  q/30/,
  'fe92b830cb9b3250a109a7306cf7bc3869bdd9419730d1e66f678bfd39d4fae1',
  'e51d60b7f0c041064204bef0f2480019c484bf60c785516cfe5a64b883eb3dc9');

++$test;
testFilter('input.miff', 'Sharpen',
  q/0.0x1.0/,
  'c8f6fdd8b866869884984a3abb960f030de09fa3c256d511833b454bb7580264',
  '7dbc4adf117dccb5eff1c05b4f0a991703f3c99166aa8cf78db4dcf7e1e918fb');

++$test;
testFilter('input.miff', 'Shave',
  q/10x10/,
  '1f3cb0be4e297e1fc20ee7b2fedf7ea6d4c8b5b91856e4f7be44ddd278b8ff81');

++$test;
testFilter('input.miff', 'Shear',
  q/10/,
  'fdaa73066e3d6ce44921aba2d64673ca931a1edd7c7b74aa9b7f87180cff79d3',
  '04a47e8e7af69f94c2ce226511ce1b9b6b926ed4d27f9ee471655a9dcbf51a89');

++$test;
testFilter('input.miff', 'Solarize',
  q/60/,
  '1a38bb91dbf862e46fc98be85e2ebb540de35da01fa69bebb899ee8b2c440a52',
  '70eb168ab1fb62363a2353fca5b3565b162ad26e383bf3632993e0ee595b34b8');

++$test;
testFilter('input.miff', 'Swirl',
  q/60/,
  '270e1d5da49b7c47b2cd1b5827890f1c0ba9ba039827a45169881537ab96c36d',
  '04fc9c3b7b0edb7ded34f516b73893dfff158e3e33714d01f2e61df015bdf0d0');

++$test;
testFilter('input.miff', 'Threshold',
  q/128/,
  'b2c86c8e0f4be2f9cc61e848ab0910ff36c4b1c923e439c9ddcbd4eba9998100',
  '16cfbd3c201b8bd388bd5a25b941736c64874bd165e2c17997452aad49a7eca8');

++$test;
testFilter('input.miff', 'Trim',
  q/0/,
  'a5d122b2b7a0944d835243afd4a81815bbea5bc1ae6c3a6ba79301662b4474a1');

++$test;
testFilter('input.miff', 'Wave',
  q/25x157/,
  'cc4cd6466273a0a0212cd29e0b628d52068dd375e2d7e641f26c36d348ca07d0',
  '2b349dd31b1ad4e304abebe4e2f75fff9fdb60a48f951884e81661c46ad1726a');

++$test;

1;
