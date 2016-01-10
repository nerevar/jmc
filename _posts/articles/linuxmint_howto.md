---
layout: article
title: "Запуск JMC под Linux Mint"
category: "articles"
tags:
description: "и сохранить девственность"
---
{% include JB/setup %}

# Запуск JMC в Linux Mint

Автор: [Mell](https://github.com/mell-inc)

Итак, вы только что прибыли в страну Linux на станцию Mint и начинаете осматриваться.
Нищие плюют вам в след! Мясники закрывают лавки у вас перед носом!
Местные вертихвостки смеются, глядя на вас, а стражники требуют с вас кодировку CP1251!

Всего этого можно избежать, если будете следовать простым советам.

#### Решение проблемы с MFC42.DLL:
Если при запуске Жабы у вас появляется ошибка вида:
`err:module:import_dll Library MFC42.DLL (which is needed by L"Z:\\home\\mell\\mell\\jmc\\jmc.exe") not found`
- `sudo apt-get install winetricks`
- `winetricks mfc42`
- удалить из папки тот файл что ты прислал
- `wine jmc.exe`

#### Решение проблемы с кодировкой CP1251
- `sudo gedit /var/lib/locales/supported.d/ru`
- добавить строчку `ru_RU.CP1251 CP1251`
- закрыть с сохранением
- `sudo locale-gen`
- отныне запускать жабу командой `LC_ALL=ru_RU.cp1251 wine jmc.exe`
- Options → Font → Courier
