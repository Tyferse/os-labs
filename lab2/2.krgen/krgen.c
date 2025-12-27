// Простая утилита для генерации картинки крестика
// Генерирует картинку в xpm-формате, а затем использует
// утилиту convert() из состава пакета imagemagick 
// для преобразования картинки в желаемый пользователем формат

#include <stdio.h>      // fputs(), fprintf(), remove()
#include <string.h>     // strcpy(), strcat()
#include <stdlib.h>     // malloc()
#include <unistd.h>     // fork(), exec()
#include <sys/wait.h>   // wait()

// Описание цвета: символ, которым цвет описывается
// и сам цвет в формате строке "RRGGBB"
typedef struct color_desc
{
	char  color_symb;
	char  color[7];
} color_desc_t;

// Создать XPM-файл
void make_xpm_file(const char* fname, 
				   const color_desc_t* colors, size_t colors_size,
				   char** image, size_t image_width, size_t image_height)
{
	FILE* mfile = NULL;
	if (image_width <= 0 || image_height <= 0 || colors_size <= 0)
		return;
	mfile = fopen(fname,"w");
	// Запишем картинку в формате XPM в файл
	fputs("/* XPM */\nstatic char *xpm_image[] = {\n",mfile);
	fprintf(mfile,"\"%lu %lu %lu 1\",\n",image_width,image_height,colors_size);
	for (int i = 0; i < colors_size; ++i)
		fprintf(mfile,"\"%c c #%s\",\n",colors[i].color_symb,colors[i].color);
	for (int i = 0; i < image_height; ++i) {
		if (i != image_height-1)
			fprintf(mfile,"\"%s\",\n",image[i]);
		else
			fprintf(mfile,"\"%s\"\n",image[i]);
	}
	fputs("};\n",mfile);
	fclose(mfile);
}

// Запустить внешнюю программу и подождать ее завершения
int run_and_wait(char* name, const char* arg1, const char* arg2)
{
	int pid, status;
	if ((pid = fork()) < 0) {
		perror("fork");
		return -1;
	} else if (!pid) {
		// дочерний процесс
		execlp(name, name, arg1, arg2, (char*)NULL);
		perror("exec");
		exit(-1);
	}
	// родительский процесс
	wait(&status);
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return -2;
}

int main(int argc, char** argv)
{
	char* fname = NULL;
	int ret = 0;
	color_desc_t colors[] = {{'`',"ffffff"},
					         {'.',"00ff00"},
						     {'#',"ff0000"},
						     {'a',"000000"},
						     {'b',"0000ff"}
						    };
	size_t colors_size = sizeof(colors)/sizeof(color_desc_t);
	char** image;
	int image_width = 0;
	int image_height = 0;
	
	// Если не указаны параметры - выведем
	// правила использования приложения и вернем ошибку
	if (argc < 3 || !(image_width = atoi(argv[2]))) {
		puts("Usage: krgen image_name width");
		return 1;
	}
	image_height = image_width;
	image = (char**)malloc(image_height*sizeof(char*));
	
	// Заполним картинку
	for (int i = 0; i < image_height; ++i) {
		image[i] = (char*)malloc(sizeof(char)*(image_width+1));
		for (int j = 0; j < image_width+1; ++j) {
			if (j == image_width)
				image[i][j] = '\0';
			else if (i < image_height/3 && j < image_width/3)
				image[i][j] = '`';
			else if (i > 2*image_height/3 && j < image_width/3)
				image[i][j] = 'a';
			else if (i < image_height/3 && j > 2* image_width/3)
				image[i][j] = '#';
			else if (i > 2*image_height/3 && j > 2* image_width/3)
				image[i][j] = 'b';
			else
				image[i][j] = '.';
		}
	}
	// Создадим XPM-файл с картинкой
	fname = (char*)malloc(strlen(argv[1])+5);
	sprintf(fname,"%s.%s",argv[1],"xpm");
	make_xpm_file(fname,colors,colors_size,image,image_width,image_height);
	// Преобразуем файл в нужный формат
	ret = run_and_wait("convert", fname, argv[1]);
	if (!ret)
		puts("Image generated succesfully");
	else if (ret < 0)
		puts("Failed to execute converter!");
	else
		printf("error executing converter: %d\n",ret);
	// Удалим файл картинки xpm
	remove(fname);
	// Почистим динамически выделенную память
	free(fname);
	for (int i = 0; i < image_height; ++i)
		free(image[i]);
	free(image);
	return 0;
}