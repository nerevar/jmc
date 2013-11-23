---
layout: page
title: "Статьи"
description: ""
---
{% include JB/setup %}

{% for post in site.categories.articles %}
## {{ post.title }}

{% unless post.tags == empty %}
<div class="row">
    <ul class="tag_box inline">
        <li><i class="icon-tags" title="Теги страницы"></i></li>
        {% assign tags_list = post.tags %}
        {% include JB/tags_list %}
    </ul>
</div>
{% endunless %}

{{ post.date | date: "%d.%m.%Y" }}

{{ post.content | strip_html | truncatewords:50 }}

[Подробнее]({{ BASE_PATH }}{{ post.url }})

---

{% endfor %} 
