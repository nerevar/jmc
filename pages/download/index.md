---
layout: page
title: "Загрузки"
description: ""
---
{% include JB/setup %}

{% for post in site.categories.downloads %}
  {{ post.date | date: "%d.%m.%Y" }}
  {{ post.content }}
  [Обсуждение]({{ BASE_PATH }}{{ post.url }})

---
{% endfor %}