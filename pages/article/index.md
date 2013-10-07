---
layout: page
title: "Статьи"
description: ""
---
{% include JB/setup %}

{% for post in site.categories.articles %}
## {{ post.title }}

{{ post.date | date: "%d.%m.%Y" }}

{{ post.content | strip_html | truncatewords:50 }}

[Подробнее]({{ BASE_PATH }}{{ post.url }})

---

{% endfor %}