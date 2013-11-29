---
layout: article
title: "Глобальные триггеры в eJMC"
category: "articles"
tags: ["js", "скрипты", "триггеры", "eJMC"]
description: ""
---
{% include JB/setup %}

# Глобальные триггеры в eJMC

Автор: [Brullek](https://github.com/brullek)

Данная статья основывается на статье [Скриптинг в JMC](http://nerevar.github.io/jmc/articles/jmc-scripts/).

Мне всегда не хватало глобальных триггеров в JMC (встроенные в версии 3.27 работали мега криво) , поэтому я вынужден был их написать сам через скриптинг.

Файлик `lib/eJMC/Vars/eJMC.Actions.js` отвечающий в рамках моего eJMC за глобальные переменные:

{% highlight javascript %}

eJMC.Actions = {}; // объект хранящий в себе все глобальные тигеры

eJMC.Actions.Init = function () {
    eJMC.Actions.Buffer = new Array();
    eJMC.Actions.Event = new Array();
    eJMC.Actions.Action = new Array();
}

eJMC.Actions.Init();

eJMC.Actions.Load = function () {
    eJMC.Actions.Init();
    //загрузка всех глобальных триггеров в ОЗУ из файла
    eJMC.Actions.Buffer = FileSystem.ReadFileToArray(FileSystem.SettingsDir + "triggers_global.set");
    // Обрабатываем полученныее строки:
    var regexp = /\{(.*)\}\{(.*)\}/;
    for (var i = 1; i <= eJMC.Actions.Buffer[0]; i++) {
        if (regexp.test(eJMC.Actions.Buffer[i])) {
            var result = regexp.exec(eJMC.Actions.Buffer[i]);
            eJMC.Actions.Event[i - 1] = result[1];
            eJMC.Actions.Action[i - 1] = result[2];
        }
    }
    //eJMC.Actions.List();
    return 1;
}


eJMC.Actions.Add = function (str) {

    var regexp = /\{(.*)\}\{(.*)\}/;

    if (regexp.test(str)) {
        var result = regexp.exec(str);
        eJMC.Actions.Event.push(result[1]);
        eJMC.Actions.Action.push(result[2]);
        return 1;

    }

    return 0;
}

eJMC.Actions.Save = function () {
    FileSystem.EraseFile(FileSystem.SettingsDir + "triggers_global.set");
    for (var i = 0; i < eJMC.Actions.Event.length; i++) {
        eJMC.Actions.Buffer[i] = "{" + eJMC.Actions.Event[i] + "}{" + eJMC.Actions.Action[i] + "}";

        FileSystem.WriteLine(FileSystem.SettingsDir + "triggers_global.set", eJMC.Actions.Buffer[i]);
    }
}


eJMC.Actions.Execute = function (mud_event) {

    for (var i = 0; i < eJMC.Actions.Event.length; i++) {

        var Quote_Event = eJMC.Regexp.Quote(eJMC.Actions.Event[i]);
        var pattern_str = Quote_Event.replace(/%\d+/g, "(.*)");
        var npattern_str = pattern_str.replace(/%s/g, "\\s*");
        var pattern = new RegExp(npattern_str + "(.*)$", "");

        //	var Quote_Action = eJMC.Regexp.Quote(eJMC.Actions.Action[i])
        var replaces = eJMC.Actions.Action[i].replace(/%(\d+)/ig, "$$$1")


        //вырезаем цветовые вставки и разделители команд
        //[1;36mТебе лучше встать на ноги!
        var quote_mud_event = replace(mud_event, /\[(\d;)?(\d)+m/g, '');


        quote_mud_event = pattern.exec(quote_mud_event);

        if (quote_mud_event == null) {
            //Строка не содержит события, переходим к следующему событию.
            continue;
        }
        // Иначе начинает реализовывать действие

        var result = quote_mud_event[0].replace(pattern, replaces);

        if (quote_mud_event[0] != result) {

            eJMC.Parse(result);
            return 1;
        }

    }

    return 0;

}

eJMC.Actions.List = function () {

    echo(eJMC.Color.Input["grey"] + "###### СПИСОК ГЛОБАЛЬНЫХ ТРИГГЕРОВ ######" + eJMC.Color.Input["default"]);

    for (var i = 0; i < eJMC.Actions.Event.length; i++) {
        echo(eJMC.Color.Input["grey"] + i + eJMC.Color.Input["default"] + ". Событие: \"" + eJMC.Color.Input["lightgreen"] + eJMC.Actions.Event[i] + eJMC.Color.Input["default"] + "\" = Действие: \"" + eJMC.Color.Input["lightred"] + eJMC.Actions.Action[i] + eJMC.Color.Input["default"] + "\"");
    }

    echo(eJMC.Color.Input["grey"] + "###### =========================== ######" + eJMC.Color.Input["default"]);
}

eJMC.Actions.Delete = function (id) {
    if (id == null) {

        echo("Данная команда имеет параметр: номер удаляемого триггер.");
        echo("Пример: #eJMC.Actions.Delete(2)");
        echo("Примечание: список объявленных глобальных триггеров доступен по команде #eJMC.Actions.List()");
        return 0;
    }

    //Если триггер еще не объявлен - то игнорируем
    if (eJMC.Actions.Event[id] == null) {
        echo("Глобальнй триггер под номером " + id + " еще не объявлен. Смотрите #eJMC.Actions.List()");
        return 0;
    }

    //DEBUG
    debug("id: " + id);
    //Реализуем удаление.
    var Event = new Array();
    var Action = new Array();

    for (var i = 0; i < eJMC.Actions.Event.length; i++) {

        if (i != id) {
            Event.push(eJMC.Actions.Event[i]);
            Action.push(eJMC.Actions.Action[i]);
        }
    }

    eJMC.Actions.Init();

    eJMC.Actions.Event = Event;
    eJMC.Actions.Action = Action;

}

{% endhighlight %}

Управление в рамках моего eJMC происходит по средствам простых команд:

{% highlight javascript %}
1. Просмотр объявленных глобальных триггеров:
    #eJMC.Actions.List();

2. Добавление нового глобального триггера: 
    // синтаксис устарел, переписать
    #eJMC.Actions.Add("{Жалкий БАРАТУР ползает у твоих ног}{пнуть бару}");

3. Удаление глобального триггера:
    #eJMC.Actions.Delete(1);

{% endhighlight %}


Пример моего файла хранящего глобальные тгрггеры:

{% highlight javascript %}

{^Ты получаешь %1 очк%2 опыт%3}{взять все.арт труп; смотр}
{^Ты не получаешь опыта.}{взять все.арт труп; смотр}
{^%s     (Артефакт) %1 валЯетсЯ здесь.}{взять арт}
{ %s(СветитсЯ) (Артефакт) %1 мерцает здесь.}{взять 1.арт}
{ %s (Артефакт) %1 мерцает здесь.}{взять 1.арт}
{ %s(Артефакт) %1 лежит у твоих ног.}{взять 1.арт}

{% endhighlight %}
