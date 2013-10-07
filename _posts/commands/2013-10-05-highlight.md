---
layout: command
title: "highlight"
description: ""
category: "commands"
tags: []
---
{% include JB/setup %}

Syntax:  
`#highlight {цвет} {строка}`

Описание: Все строки, исходящие из MUD, содержащие заданный участок, будут подсвечиваться указанным цветом.

Если вы хотите подсветить строку, содержающую заданный участок, полностью - используйте переменные `%0-%9` для указания меняющихся слов строки.

Параметр `цвет` может быть следующим:  
- reverse, bold, blink, faint, italic, или число от 1 до 8 (соответствует цветам основной палитры).

Также вы можете напрямую использовать название цвета или эффекта или их комбинации. Например, вы можете задать `{red, bold}`.

Доступные названия:  
red, blue, cyan, green, yellow, magenta, white, grey, black, brown, charcoal, light red, light blue, light cyan, light magenta, light green, b red, b blue, b cyan, b green, b yellow, b magenta, b white, b grey, b black, b brown, b charcoal, b light red, b light blue, b light cyan, b light magenta, b light green, bold, faint, blink, italic, reverse

Примеры:  
`#highlight {light red, italic} {тяжело ранил вас}`  
Когда кто-либо тяжело ранит вас, строка с информацией об этом будет показана наклонным шрифтом ярко-красного цвета.

`#highlight {yellow} {^%0 дал вам %1 монет}`  
Теперь строки типа "Вася дал вам 1000 монет" будут показываться желтым цветом.

См. также: [#substitute](#substitute), [#multihightlight](#multihightlight)
