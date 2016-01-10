---
layout: article
title: "Как запустить JMC под Linux Mint"
category: "articles"
tags:
description: "и сохранить девственность"
---
{% include JB/setup %}

Автор: [Mell](https://github.com/mell-inc)

Итак, вы только что прибыли в страну Linux на станцию Mint и начинаете осматриваться.

Нищие плюют вам в след! Мясники закрывают лавки у вас перед носом!
Местные вертихвостки смеются, глядя на вас, а стражники требуют с вас кодировку CP1251!

Всего этого можно избежать, если будете следовать простым советам.

#### Решение проблемы с недостающей в вашем Wine библиотеки MFC42.DLL:
- `sudo apt-get install winetricks`
- `winetricks mfc42`
- `wine jmc.exe`


#### Решение проблемы с кодировкой CP1251:
- `sudo gedit /var/lib/locales/supported.d/ru`
- добавить строчку `ru_RU.CP1251 CP1251`
- закрыть с сохранением
- `sudo locale-gen`
- отныне запускать жабу командой `LC_ALL=ru_RU.cp1251 wine jmc.exe`
- Поменять шрифт в настройках Жабки: Options → Font → Courier (или Ubuntu Mono, он-таки более шрифт)
