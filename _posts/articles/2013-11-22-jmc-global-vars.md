---
layout: article
title: "Глобальные переменные скриптов в eJMC"
category: "articles"
tags: ["js", "скрипты", "сохранение переменных скриптов в файлы", "eJMC"]
description: ""
---
{% include JB/setup %}

# Глобальные переменные в eJMC

Автор: [Brullek](https://github.com/brullek)

Данная статья основывается на статье [Скриптинг в JMC](http://nerevar.github.io/jmc/articles/jmc-scripts/).

Мне всегда не хватало глобальных переменных в JMC (встроенные в версии 3.27 работали мега криво) , поэтому я вынужден был их написать сам через скриптинг.

Файлик `lib/eJMC/Vars/eJMC.Vars.js` отвечающий в рамках моего eJMC за глобальные переменные:

{% highlight javascript %}

// Конструктор переменных, реализует get, если указан только параметр name;
// set/update - если указан параметр value!='';
// unset - если value=='';
eJMC.Vars = function(name, value){
	if (name==null) {
		_help("Данная команда имеет параметры: name, [value].");
		_help("Если указан только name - вернет значение переменной, или сообщит, что таковая отсутствует");
		_help("Если указан value - обновит значение переменной, или создаст ее, в случе отсутствия");
		_help("Если value=\"\" (пустая строка) - удалит переменную, в случае ее существования");
		_help("Пример создания переменной: #eJMC.Vars(\"Weapons\", \"axe\")");
		_help("Пример получения значения пепременной: #eJMC.Vars(\"Weapons\")");
		_help("Пример удаления пепременной: #eJMC.Vars(\"Weapons\",\"\")");
		_help("Примечание: список объявленных глобальных переменных доступен по команде #eJMC.Vars.List()");
		return 0;
	} else 
	
	if (value==null) {
		//Вернуть значение переменной
		var varitable = eJMC.Vars.Get(name);
			if (varitable){
			// Пока выводим только на экран, не возвращая... код находиться в разработке...
			echo(varitable);
		}
	} else
	
	if (value==""){
		//Удалить переменную
		eJMC.Vars.Delete(name);
	} else
		//Создать или обновить переменную
		eJMC.Vars.Add(name, value);
}


// Файл хронящий глобальные переменные
eJMC.Vars.File = FileSystem.SettingsDir  + "vars_global.set";

eJMC.Vars.Init = function(){
	eJMC.Vars.Buffer = new Array();
	eJMC.Vars.Names = new Array();
	eJMC.Vars.Values = new Array();
}

eJMC.Vars.Init();

eJMC.Vars.Load = function(obj){
	// Обнуление текущих настроек в ОЗУ
	eJMC.Vars.Init();
	
	//загрузка всех глобальных переменных в ОЗУ из файла
	eJMC.Vars.Buffer = FileSystem.ReadFileToArray(eJMC.Vars.File);
	
	// Обрабатываем полученныее строки:
	// Переменные храняться в файле в формате:
	// {key1}{value1}
	// {key2}{value2}
	// ...
	// {keyN}{valueN}
	var regexp = /\{(.*)\}\{(.*)\}/;
	for (var i=1; i<=eJMC.Vars.Buffer[0]; i++){
		if (regexp.test(eJMC.Vars.Buffer[i])) {
			var	result = regexp.exec(eJMC.Vars.Buffer[i]);
			eJMC.Vars.Names[i-1] = result[1];
			eJMC.Vars.Values[i-1] = result[2];
		}
	}
	
	//eJMC.Vars.List();
	return 1;
}
	
// Добавление или обнволение значения переменной	
eJMC.Vars.Add = function(name, value){
	
	if (typeof(name)=="string"){	
		// Получить переменную по имени, что бы 
		var position = eJMC.Vars.GetId(name);
	} else {
		var position = name;
		
	}
	
	if (position!==false && eJMC.Vars.Names[position]!=null){
		
		// Переменная существует, ее нужно обновить
		eJMC.Vars.Values[position] = value;
		return true;
		
	} else {
		// Добавляем новую переменную
		
		if (typeof(name)!=="string"){
			debug("Ошибка: переменная должна иметь строковое имя!");
			return false;
		}
		eJMC.Vars.Names.push(name);
		eJMC.Vars.Values.push(value);
		return true;
	}
	return false;
}
	
// Получение ID переменной по идентификатору	
eJMC.Vars.GetId = function(name){

	if (typeof(name)=="string"){

		for (var i=0; i<eJMC.Vars.Names.length; i++){
			if (eJMC.Vars.Names[i]==name){
				debug("i=" + i);
				return i;
			}

		}
	} else 

	if (typeof(name)=="number"){
			if (eJMC.Vars.Names[name]==null){
				return false;
			} else
				return i;
	}
	return false;
}

// Получние значения переменной
eJMC.Vars.Get = function(id){

	if (typeof(id)=="string"){

		for (var i=0; i<eJMC.Vars.Names.length; i++){
			if (eJMC.Vars.Names[i]==id){
				return eJMC.Vars.Values[i]
			}

		}
	} else 

	if (typeof(id)=="number"){
			if (eJMC.Vars.Names[id]==null){
				return false;
			} else
				return eJMC.Vars.Values[id];
	}
	return false;
}


// Сохранение глобальных переменных на диск
eJMC.Vars.Save = function(){
	FileSystem.EraseFile(eJMC.Vars.File);
	for (var i=0; i<eJMC.Vars.Names.length; i++){
		eJMC.Vars.Buffer[i] = "{" +eJMC.Vars.Names[i]+"}{"+eJMC.Vars.Values[i]+"}";

		FileSystem.WriteLine(eJMC.Vars.File, eJMC.Vars.Buffer[i]);
	}
}

// Просмотр текущих глобальных переменных
eJMC.Vars.List = function(){

	echo(eJMC.Color.Input["grey"] + "###### СПИСОК ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ ######" + eJMC.Color.Input["default"]);
	
	for(var i=0; i<eJMC.Vars.Names.length; i++){
		echo(eJMC.Color.Input["grey"] + i + eJMC.Color.Input["default"]  + ". Переменная: \"" + eJMC.Color.Input["lightgreen"] + eJMC.Vars.Names[i] + eJMC.Color.Input["default"] +  "\" = Значение: \""  + eJMC.Color.Input["lightred"] + eJMC.Vars.Values[i] + eJMC.Color.Input["default"] + "\"");	
	}
	
	echo(eJMC.Color.Input["grey"] + "###### =========================== ######" + eJMC.Color.Input["default"]);
}

// Удаление переменной
eJMC.Vars.Delete = function (id){

	if (id==null){
		
		_help("Данная команда имеет параметр: номер удаляемой переменной или имя.");
		_help("Пример: #eJMC.Vars.Delete(2)");
		_help("Пример: #eJMC.Vars.Delete(\"align\")");
		_help("Примечание: список объявленных глобальных переменных доступен по команде #eJMC.Vars.List()");
		return false;
	}
	
	
	if (typeof(id)=="string"){
		// Удаляем переменную по имени
		var id = eJMC.Vars.GetId(id);
	} 
	
		
	if (eJMC.Vars.Names[id]==null){
		debug("Глобальая переменная под номером " + id + " еще не объявлен. Смотрите #eJMC.Vars.List()");
		return false;
	}	
		
		//DEBUG
		debug("id: " + id);
		
		//Реализуем удаление.
		var Names = new Array();
		var Values = new Array();
		
		for (var i=0; i<eJMC.Vars.Names.length; i++) {
			
			if (i!=id){
				Names.push(eJMC.Vars.Names[i]);
				Values.push(eJMC.Vars.Values[i]);
			}
		}
	
		eJMC.Vars.Init();
		
		eJMC.Vars.Names = Names;
		eJMC.Vars.Values = Values;
			
	}

{% endhighlight %} 

