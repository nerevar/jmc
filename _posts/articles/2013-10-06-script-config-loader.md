---
layout: article
title: "Сохранение переменных скриптов в отдельный файл"
category: "articles"
tags: ["js", "скрипты", "сохранение переменных скриптов в файлы"]
description: ""
---
{% include JB/setup %}

# Сохранение переменных скриптов в отдельный файл

Автор: [Nerevar](https://github.com/nerevar)

При открытии jmc создаёт новый контект javascript, при этом не сохраняя переменные, которые были объявлены до этого.  

Для того, чтобы сохранять какие-то важные значения, мы написали обёртку для загрузки/сохранения переменных в файл.  

Синтаксис использования:
{% highlight javascript %}
// Сохранить значение переменной:
CONFIG.set('HP', 350);

// Загружает весь конфиг и все глобальные переменные
CONFIG.load();

// Возвращает значение переменной ManaPoints
CONFIG.get('ManaPoints');
{% endhighlight %}

Весь исходный код:

{% highlight javascript %}
/**
 * Singleton объект конфига
 */
var CONFIG = {
    data: {}
};

/**
 * Устанавливает значение переменной variable, при этом делая её глобальной и сохраняет конфиг в файл
 */
CONFIG.set = function(variable, value) {
    this.data[variable] = value;
    this.extract(variable, value);
    this.save();
    jmc.showme(D.CYAN + "Variable '" + L.GRAY + variable + D.CYAN + "' is set to '" + L.GRAY + JSON.stringify(value) + D.CYAN + "'." + L.NORMAL);
};

/**
 * Возвращает значение переменной variable
 */
CONFIG.get = function(variable) {
    return this.data[variable] || '';
};

/**
 * Сохраняет текущие переменные в файл
 */
CONFIG.save = function() {
    write2file('settings\\configdata.js', 'CONFIG.data = ' + JSON.stringify(this.data), true);
};

/**
 * Загружает конфиг и все глобальные переменные
 */
CONFIG.load = function() {
    include('settings\\configdata.js');
    this.extractAll();
};

/**
 * Делает переменную variable глобальной, устанавливая значение value
 */
CONFIG.extract = function(variable, value) {
    eval(variable + ' = ' + JSON.stringify(value));
};

/**
 * Делает глобальными все переменные конфига и устанавливает им значения
 */
CONFIG.extractAll = function(variable, value) {
    for (var i in this.data) {
        this.extract(i, this.get(i));
    }
};

/*
 * Записывает текстовую строчку в указанный файл
 */
function write2file(filename, textline, clearfile) {
    if (fso.FileExists(filename)) {
        if (clearfile) {
            var file = fso.DeleteFile(filename);
            var file = fso.CreateTextFile(filename, true);
        } else {
            var file = fso.OpenTextFile(filename, 8);
        }
    } else {
        var file = fso.CreateTextFile(filename, true);
    }

    file.WriteLine(textline);
    file.Close();
    delete file;
}
{% endhighlight %}

