---
layout: page
title: "Команды JMC"
description: ""
---
{% include JB/setup %}

{% for command in site.categories.commands reversed %}

<h1 class="commands">#{{ command.title }}</h1>
{{ command.content }}  
[Обсуждение]({{ BASE_PATH }}{{ command.url }})  

---
{% endfor %}
