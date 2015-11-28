---
layout: article
title: "Цветовая схема для конфигов jmc в Sublime Text 3"
category: "articles"
tags: ["конфиг"]
description: ""
---
{% include JB/setup %}

# Цветовая схема для конфигов jmc в Sublime Text 3

Автор: [Nerevar](https://github.com/nerevar)

Про то, как создавать вообще [свои схемы языков или конфигов](http://docs.sublimetext.info/en/latest/extensibility/syntaxdefs.html)

#### По шагам, под windows:
- Установить пакет `AAAPackageDev` через Ctrl+Shift+P → Install package → AAAPackageDev
- Tools → Packages → Package Development → New Syntax Definition
- Сохранить файл как `.YAML-tmLanguage`
- Чтобы скомпилировать в `tmLanguage` → нажать F7
- Перезагрузить редактор

Получившийся в итоге конфиг:

    ---
    name: JMC set
    scopeName: config.jmc
    fileTypes: [set]
    uuid: eda6a7b4-f91e-47b4-beb3-f762dbcfcbc4

    patterns:
    - comment: Comments
    name: comment.line.jmc
    match: $\#(\#|nop)(.+)

    - comment: JMC commands
    name: keyword.control.jmc
    match: \#([A-Za-z]+)

    - comment: Substitute %1, %0
    name: substitute.other.jmc
    match: (\%+)(\d+)
    captures:
        '2': {name: constant.numeric.jmc}

    - comment: Variables like $variable, $TM_SELECTION...
    name: keyword.other.jmc
    match: \$([A-Za-z][A-Za-z0-9_]+)
    captures:
        '1': {name: variable.other.jmc}

    - comment: Escape colors
    name: keyword.other.jmc
    match: (\e.+?m)
    captures:
        '1': {name: constant.character.escape.jmc}

    - name: variable.complex.jmc
    begin: '(\{)'
    beginCaptures:
        '1': {name: keyword.other.jmc}
    end: '(\})'
    endCaptures:
        '1': {name: keyword.other.jmc}
    patterns:
    - include: $self
    - name: support.other.jmc
        match: .


Благодаря чему конфиги для jmc будут выглядеть вот так:

![](https://raw.githubusercontent.com/nerevar/jmc/gh-pages/uploads/2015-11-28%2023-34-28%20Sublime%20Text%20-%20JMCconfig%20syntax%20hightlight.png)
