---
layout: page
title: "О разработке самого клиенте"
description: ""
---
{% include JB/setup %}

[Назад]({{ BASE_PATH }}/pages/developers/)

## IDE, Сборка
Для разработки, сборки, компиляции, отладки - используется среда Microsoft Visual C++ 6 (Visual C++ 6.0 (commonly known as VC6), which included MFC 6.0, was released in 1998)
Студия работает под Windows XP и Windows 7.

Шаги установки:

- Скачать образ <http://rutracker.org/forum/viewtopic.php?t=1247466>
- Установить сначала просто MicroSoft Visual C++ 6
- Скопировать локально во временную папку установщик SP5 (VC6_SP5) для последующей модификации
- В файле sp598ent.stf:
    - заменить `13 Group 28 36 38 29 30 32 26 27 14 25 16 17 20 18 19 15 39 21 22 24 23 43`  
    на `13 Group 28 38 29 30 32 26 27 14 25 16 17 20 18 19 15 39 21 22 24 23 43`  
    при этом стараясь менять только сами чиселки, не меняя отступов/переносов/форматирования
    - удалить строки:  
        `36 Depend "27 ? : 37" `  
        `37 IsWin95 CustomAction "sp598ent.dll,CheckForMDAC"`
- Запустить `setupsp5.exe` от имени администратора и установить
- Руссификатор устанавливать НЕ НУЖНО (!)

[Более детально, на английском](http://answers.microsoft.com/en-us/windows/forum/windows_7-windows_programs/installing-visual-studio-6-on-windows-7/fe63b96d-0005-4058-96a6-3d78d8041556)

## Исходный код

Скачать последнюю версию исходников проекта можно с основной страницы проекта <https://github.com/nerevar/jmc> - кнопка *Download ZIP*

Либо более предпочтительный способ - клонировать репозиторий и разрабатываться локально, используя все прелести git'а:  
`git clone https://github.com/nerevar/jmc.git`

---

{% include JB/comments %}
