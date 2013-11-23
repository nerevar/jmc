---
layout: article
title: "Скриптинг в JMC. Базовые методы и настройки - eJMC"
category: "articles"
tags: ["js", "скрипты", "eJMC"]
description: ""
---
{% include JB/setup %}

# Скриптинг в JMC

Автор: [Brullek](https://github.com/brullek)

Оговорюсь сразу, что я не знаю язык JS :) И все строки кода были написаны без представления работы языка JS.

В JMC3.5 в каталоге `./JMC/settings` можно создать файл `commonlib.scr` поместив в него JS код, который будет исполняться при запуске скриптового движка во всех профилях. 

Приведу примеры реализации своих JS скриптов:

{% highlight javascript %}

	// Начинаю с настройки путей и подгрузки FileSystemObject, который будет инклудить весь пользовательский JS код.
	FileSystem = {};
	
	// Здесь у меня лежит жаба. В windows слеши приходится экранировать, и двойные слеши тут не случайны
	FileSystem.BinDir = "C:\\JMC\\JMC\\";
	
	// Каталог, где у меня лежат файлы с текстами песен, используется в eJMC.MusicPlayer
	// Стоило бы вынести в другое место, но у меня так сложилось, что все пути назначаются именно в этом скрипте,
	// Что бы при переезде жабы на другой комп, править пришлось
	только этот файл.
	FileSystem.SongsDir = FileSystem.BinDir + "songs\\";
	
	// Каталог с конфигами
	FileSystem.SettingsDir = FileSystem.BinDir + "settings\\";
	
	// Каталог с логами
	FileSystem.LogDir = FileSystem.BinDir + "logs\\";
	
	// Создание объекта, по работе с файловой системой.
	// Возможно нужно убдет зарегистрировать какую-то dll, перед использованием.
	FileSystem.File =  new ActiveXObject("Scripting.FileSystemObject");
	
	// Режимы работы с файлами: чтение, запись, дозапись
	FileSystem.mode = { 
	"Read" : 1,
	"Write" : 2,
	"Append" : 8
	}

	// Функция include, служит для подгрузки дополнительного кода
	// Имеет один входной строковый параметр sFileName - путь и имя подгружаемого файла, относительно FileSystem.SettingsDir
	FileSystem.Include = function(sFileName){
	
	// Открытие файла на чтение
	var Stream = FileSystem.File.OpenTextFile(FileSystem.SettingsDir + sFileName, FileSystem.mode.Read);
	
	// Подгрузка содержимого файла в скриптовой движок JMC
	jmc.Eval(Stream.ReadAll());
	
	// Закрытие файла
	Stream.Close();
	} 
	
	// Дальше я подгружаю различные хелперы и, собсвенно,  eJMC.
	// У меня eJMC - объект содержащий все мои `extended`
	FileSystem.Include("lib\\functions.js")
	FileSystem.Include("lib\\FileSystem\\FileSystem.Functions.js")
	FileSystem.Include("lib\\System\\System.js")
	FileSystem.Include("lib\\eJMC\\eJMC.js")
	
{% endhighlight %}


Пример `lib/FileSystem/FileSystem.Functions.js`

{% highlight javascript %}

	// Чтение файла в переменную
	FileSystem.ReadFile = function(sFileName){
		var Stream = FileSystem.File.OpenTextFile(FileSystem.SettingsDir + sFileName, FileSystem.mode.Read);
		var result = Stream.ReadAll();
			Stream.Close();
			return result;
	}
	
	// Запись строки в файл
	FileSystem.WriteLine = function(sFileName, text){
		var Stream = FileSystem.File.OpenTextFile(sFileName, FileSystem.mode.Append);
			Stream.Write(text + "\n");
			Stream.Close();
	}
	
	// Очистка файла
	FileSystem.EraseFile = function(sFileName){
		var Stream = FileSystem.File.OpenTextFile(sFileName, FileSystem.mode.Write);
			//Stream.Write(text + "\n");
			Stream.Close();
	}
	
	// Функция возвращает массив строк, элемент с индексом 0 хранит в себе количество строк.
	// Полезно, когда нам нужно иметь построчный доступ к данным файла.
	// Например: одна строка = одно правило чего-либо.
	FileSystem.ReadFileToArray = function(sFileName){
		var Stream = FileSystem.File.OpenTextFile(sFileName, FileSystem.mode.Read);
		
		var result = [0];
		
		// Пока не достигнут конец файла, считываем из него данные	
		while (Stream.AtEndOfStream == false) {
			
			var str = Stream.ReadLine();
			
			//Добавляет строку в массив, а в result[0] возвращает размер получившегося массива
			result[0] = result.push(str);
		}
			// result[0] = размерности массива, но счет в массиве начинается не с 1 а с 0 )))
		result[0] = result[0]-1;
			
			// Закрываем файл и возвращаем результат
			Stream.Close();
			return result;
		
	}


{% endhighlight %}

Т.к. мой JS код мада находиться на вечной стадии размышлений, тестов и допилов, пример своего `lib/functions.js` и `lib/System/System.js` не привожу.

Пример `lib/eJMC/eJMC.js`

{% highlight javascript %}

	var eJMC = {}; //Создаем объект Extended JMC
	
	// Игровой профиль, беру из жабы
	CharName = jmc.Profile;
	
	// Начинаю инклудить всевозмодный код
	FileSystem.Include("lib\\eJMC\\eJMC.Functions.js")
	FileSystem.Include("lib\\eJMC\\OnIncoming\\eJMC.OnIncoming.js")
	FileSystem.Include("lib\\eJMC\\OnTimer\\eJMC.Timers.js")
	FileSystem.Include(CharName + "\\lib\\profile.js");
	FileSystem.Include("lib\\eJMC\\Bots\\eJMC.Bots.js");
	FileSystem.Include("lib\\eJMC\\Logon\\eJMC.Logon.js")
	FileSystem.Include("lib\\eJMC\\Log\\eJMC.Log.js")
	FileSystem.Include("lib\\eJMC\\Chanel\\eJMC.Chanel.js")
	FileSystem.Include("lib\\eJMC\\eJMC.Colors.js")
	FileSystem.Include("lib\\eJMC\\OnInput\\eJMC.OnInput.js")
	FileSystem.Include("lib\\eJMC\\Subs\\eJMC.Subs.js")
	FileSystem.Include("lib\\eJMC\\Action\\eJMC.Action.js")
	FileSystem.Include("lib\\eJMC\\Vars\\eJMC.Vars.js")
	FileSystem.Include("lib\\eJMC\\MusicPlayer\\eJMC.MusicPlayer.js")
	
	
	// Обработчик событий при срабатывании таймера
	eJMC.OnTimer = function(){
		
		// При срабатывании таймера, нам нужно знать id таймера, который сработал, т.к. можно запустить много таймеров
		id_timer = jmc.Event;
		
		// Собсвенно, исполняем код, в соответсвии с id таймера.
		// Описание смотрите в файле `lib/eJMC/OnTimer/eJMC.Timers.js`
		eJMC.Timers.TimerExec(id_timer);
		
	}
	
	
	// Обработчик событий при перезапуске скриптового движка жабы
	eJMC.OnReLoad = function(){
		
		// Описание смотрите в `lib\\eJMC\\eJMC.Functions.js`
		eJMC.Reload();
	}

{% endhighlight %}

Пример lib\\eJMC\\eJMC.Functions.js

{% highlight javascript %}

	// Регистратор для событий жабы
	eJMC.RegisterHandler = function(event, handler){
		jmc.RegisterHandler(event , "eJMC." + handler + "()");
	};
	
	
	// Регистрирую обработчик для события, возникающего при вводе данных с клавиаттуры или при вызове метода jmc.parce()
	eJMC.RegisterHandler("Input", 		"OnInput");
	
	// Регистрирую обработчик для события, возникающего при получении данных с сервера, в жабу
	eJMC.RegisterHandler("Incoming", 	"OnIncoming");
	
	// Регистрирую обработчик для события, возникающего срабатывание запущенных таймеров
	eJMC.RegisterHandler("Timer",		"OnTimer");
	// eJMC.RegisterHandler("PreTimer",	"OnPreTimer");
	
	// Не помню что это за событие, но вроде бы запуск/перезапуск скриптового движка.
	eJMC.RegisterHandler("Load",		"OnReLoad");
	
	// Вывод строки в дополнительное окно жабы с пометкой времени.
	// Бывает нужно в отдельное окно выносить важные события и помечать когда они были
	eJMC.Output = function(str){
		jmc.Output(eJMC.Date.getTime() + str);
	}
	
	
	/*
	eJMC.Showme = function(str){
		echo(str);
	}
	*/
	
	/*
	eJMC.echo = function(str){
		echo(str);
	}
	*/
	
	// Показывает профиль жабы, с которым работает eJMC
	eJMC.ShowProfile =  function(){
		echo(eJMC.Profile);
	}
	
	// Отправка строки в JMC, как будто она была введена пользователем с клавиатуры.
	// В выводе мы будем видеть строку, как будто только что ввели ее сами
	eJMC.Parse = function(str){
		jmc.Parse(str);
	}
	
	// Отправка строки на сервер.
	// Я не помню сечас явной разницы с eJMC.Parse, но она есть :)
	eJMC.Send = function(command){
		jmc.send(command);
	}
	
	
	// Дальше разные вещи на получение времени
	eJMC.Date = {};
	eJMC.Date.getTime = function(){
		var time  = new Date();
		var hour = time.getHours();
		var minute = time.getMinutes();
		var seconds = time.getSeconds();
		
		if (hour<10) {
			hour = "0" + hour;
		}
		
		if (minute<10) {
			minute = "0" + minute;
		}
	
		if (seconds<10) {
			seconds = "0" + seconds;
		}
	
		var timeresult = hour +":" + minute + ":" + seconds + "> ";
		//return 19:02:44>
		return timeresult;
	}
	
	eJMC.Date.getDay = function(){
		var time  = new Date();
		var day = time.getDate();
		var month = time.getMonth() + 1;
		var year = time.getFullYear();
		
		if (day<10) {
			day = "0" + day;
		}
		
		if (month<10) {
			month = "0" + month;
		}
	
	
		var result = day +"-" + month + "-" + year;
		//return 12-05-2012>
		return result;
	}
	
	eJMC.Date.getDate = function(){
		var time = eJMC.Date.getTime();
		var day = eJMC.Date.getDay();
		var result = day + " " + time;
		return result
	}
	
	// Регулярные выражения, куда же без них )
	eJMC.Regexp = {};
	
	eJMC.Regexp.Quote = function(str) {
	// Эти символы следует экранировать.
	//	. \ + * ? ^ $ [ ] ( ) { } < > = ! | :
		var result = str.replace(/(\.|\\|\+|\*|\?|\[|\]|\(|\)|\{|\}|\<|\>|\=|\!|\||\:|\$)/g, "\\$1" );
		return result;
	}
	
	// Собсвенно при перезагрузке скриптового движка, следует перечитать различные данные.
	eJMC.Reload = function(){
	
		eJMC.Actions.Load();
		eJMC.Subs.Load();
		eJMC.Vars.Load();
	}
	
	// Генератор чисел, полезная штука, для написания ботов и активации произвольных событий
	eJMC.Random = function(id){
	
		var rand = Math.round(Math.random() * id);
		return rand;
		
	}
 
{% endhighlight %}

Имея обработчики для различных собыйти: входящие строки, исходящие строки, срабатывания таймеров,
мы вольны делать с этим все что захотим на JS.

Это пример, как я реализовал поддержу в JMC своих JS скриптов. Далее попробую описать различные частные
полезные штуки. Смотрите так же - [Работа с таймерами](http://nerevar.github.io/jmc/articles/ontimers/)
