---
layout: command
title: "scriptlet"
description: ""
category: "commands"
tags: []
---
{% include JB/setup %}

Syntax:  
`#scriptlet <plainscript>`

Интерпретирует `plainscript` через выбранный скриптовый движок

Примеры:  
`#scr if (jmc.getVar('hp') < 50) { jmc.showme('Убегаем!'); jmc.parse('flee'); }`  
Если в переменной `#var hp` окажется меньше 50 хп - то выводит сообщение на экран и отправляет команду убегать


См. также: [#use](#use)
