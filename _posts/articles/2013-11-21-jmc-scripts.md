---
layout: article
title: "Статья"
category: "articles"
description: ""
---
{% include JB/setup %}

# Скриптинг в JMC

В JMC3.5 в каталоге ./JMC/settings можно создать файл commonlib.scr поместив в него JS код, который будет исполняться
при запуске скриптового движка во всех профилях. 

Приведу примеры реализации своих JS скриптов:

{% highlight javascript %}

// Начинаю с настройки путей и подгрузки FileSystemObject, который будет инклудить весь пользовательский JS код.
FileSystem = {};
	
	// Здесь у меня лежит жаба. В windows слеши приходится экранировать, и двойные слеши тут не случайны
	FileSystem.BinDir = "C:\\JMC\\JMC\\";
	
	// Каталог, где у меня лежат файлы с текстами песен, используется в eJMC.MusicPlayer
	// Стоило бы вынести в другое место, но у меня так сложилось, что все пути назначаются именно в этом скрипте,
	// Что бы при переезде жабы на другой комп, править пришлось только этот файл.
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




