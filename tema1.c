#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LINE_SIZE 300
#define MAX_COMMAND_SIZE 20
#define MAX_NAME_SIZE 20
#define MAX_PATH_SIZE 100

//functie de stergere
typedef void *erase(void *);

//structura lista generica
typedef struct Node {
	void *info;
	struct Node *next;
} Node, *List;

//structura director
typedef struct Dir{
	char *name;
	struct Dir* parent;
	List head_children_files;
	List head_children_dirs;
	struct Dir* next;
} Dir;

//structura de fisier
typedef struct File {
	char *name;
	struct Dir* parent;
	struct File* next;
} File;

// dezalocare lista
void dez_list(List list, erase *del) {
	List copy;

	while(list != NULL) {
		copy = list;
		list = list->next;
		del (copy->info);
		free (copy);
	}
}

void dez_file (File *file) {
	free (file->name);
	free (file);
}

//dezalocare dir
void dez_dir(Dir *dir) {
	free(dir->name);
	dez_list(dir->head_children_files, (void *) dez_file);
	dez_list(dir->head_children_dirs, (void *) dez_dir);
	free(dir);

}

//alocare structura director
Dir *allocate_Dir() {

	Dir *directory = (Dir *)calloc(1, sizeof(Dir));
	directory->name = (char *)calloc(MAX_NAME_SIZE, sizeof(char));
	directory->parent = NULL;
	directory->next = NULL;
	directory->head_children_dirs = NULL;
	directory->head_children_files = NULL;
	return directory;

}

//alocare structura fisier
File *allocate_File() {
	File *file = (File *)calloc(1, sizeof(File));
	file->name = (char *)calloc(MAX_NAME_SIZE, sizeof(char));
	file->parent = NULL;
	file->next = NULL;
	return file;
}

//afisare nume director
void print_directoryname(void *directory) {
	printf("%s\n", ((Dir *)directory)->name);
}

//afisare nume fisier
void print_filename(void *file) {
	printf("%s\n", ((File *)file)->name);
}

//afisare lista generica
void print_list(List list, void (*print_element)(void *))
{
	List aux = list;
	//lista vida
	if (list == NULL) {
		return;
	}

	//afisez toate elementele
	for(; aux != NULL; aux = aux->next) {
		print_element(aux->info);
	}
}

//afisare lista generica pe nivel
void print_list_rec(List list, void (*print_element)(void *), int level)
{
	List aux = list;

	//lista vida
	if (list == NULL) {
		return;
	}

	//afiseaza spatii pentru forma arborescenta
	int copy = 4 * level;
	while (copy) {
		printf(" ");
		copy--;
	}

	//afisez toate elementele
	for(; aux != NULL; aux = aux->next) {
		print_element(aux->info);
	}
}

//inserare lista
void insert(List *head, void *info)
{
	
	List new_node = (List)calloc(1, sizeof(Node));
	new_node->info = info;
	new_node->next = NULL;

	//inserare inceput, lista vida
	if (*head == NULL) {
		*head = new_node;
		return;
	}
	
	//inserare final
	List last = *head;
	while (last->next != NULL) {

		last  = last->next;
	}

	last->next = new_node;
	return;

}

//compara fisier 0 == gaseste, 1 == nu
int compare_file(char *name, void *file) {
	if (strcmp(name, ((File *)file)->name) == 0) {
		return 0;
	}
	return 1;
}

//compara director 0 == gaseste, 1 == nu
int compare_directory(char *name, void *directory) {
	if (strcmp(name, ((Dir *)directory)->name) == 0) {
		return 0;
	}
	return 1;
}

//cautare in lista generica 0 == gaseste, 1 == nu
int search(List head, char *name, int (*compare_element)(char *, void *))
{
	List current = head;
	for (; current != NULL; current = current->next) {
		if (compare_element(name, current->info) == 0) {
			return 0;
		}
	}
	return 1;
}

//returneaza dirctorul cautat sau NULL
void* search_directory(List head, char *name)
{	
	List current = head;
	for (; current != NULL; current = current->next) {
		if (compare_directory(name, current->info) == 0) {
			return  current->info;
		}
	}
	return NULL;
}

//returneaza fisierul cautat sau NULL
void *search_file(List head, char *name)
{
	List current = head;
	for (; current != NULL; current = current->next) {
		if (compare_file(name, current->info) == 0) {
			return  current->info;
		}
	}
	return NULL;
}

//stergere din lista
void delete(List *head, int (*compare_element)(char *, void *), char *name, char *type, Dir *parent) {
	List aux = *head;
	List prev = NULL;

	//se sterge primul element
	if (aux != NULL && compare_element(name, aux->info) == 0) {
		*head = aux->next;
		if (strcmp (type, "dir") == 0) {
			dez_dir (aux->info);
		}
		else {
			dez_file (aux->info);
		}
		free (aux);

		return;
	}

	//se parcurge lista si se cauta elementul
	while (aux != NULL && compare_element(name, aux->info) != 0) {
		prev = aux;
		aux = aux->next;
	}

	//nu s-a gasit fisierul/directorul
	if (aux == NULL) {
		if (strcmp(type, "file") == 0) {
			printf("Could not find the file\n");
		}
		else if (strcmp(type, "dir") == 0){
			printf("Could not find the dir\n");
		}
		return;
	}

	prev->next = aux->next;
	
	if (strcmp (type, "dir") == 0) {
		dez_dir (aux->info);
	}
	else { 
		dez_file (aux->info);
	}
	free (aux);
	
}

//creare fisier in directorul parent
void touch (Dir* parent, char* name) {
	if (search(parent->head_children_dirs, name, compare_directory) == 0 ||
		search(parent->head_children_files, name, compare_file) == 0) {

		printf("File already exists\n");

		return;
	}

	//creez fisier nou
	File *new_file = allocate_File();

	//actualizez nume
	strcpy(new_file->name, name);

	//setez parintele noului fisier
	new_file->parent = parent;

	insert(&(parent->head_children_files), (void *)new_file);
}

//creare director
void mkdir (Dir* parent, char* name) {
	if (search(parent->head_children_dirs, name, compare_directory) == 0) {
		printf("Directory already exists\n");
		return;
	}

	//creez un nou director
	Dir *new_directory = allocate_Dir();
	
	//setez numele
	strcpy(new_directory->name, name);
	
	//setez parintele noului director
	new_directory->parent = parent;

	//seteez urmator pentru parinte
	parent->next = new_directory;
	
	//caut existenta directorului
	
	insert(&(parent->head_children_dirs), (void *)new_directory);

}

//afiseaza continutul directorului parinte
void ls (Dir* parent) {

	//afisez lista de directotare
	print_list(parent->head_children_dirs, print_directoryname);
	//afisez lista de fisiere
	print_list(parent->head_children_files, print_filename);
}

//stergere fisier
void rm (Dir* parent, char* name) {
	delete(&(parent->head_children_files), compare_file, name, "file", parent);
}

//stergere director
void rmdir (Dir* parent, char* name) {
	delete(&(parent->head_children_dirs), compare_file, name, "dir", parent);
}

//schimbare director
void cd(Dir** target, char *name, Dir *home) {

	List current = (*target)->head_children_dirs;
	
	//se cauta directorul
	for (; current != NULL; current = current->next) {
		
		if (compare_directory(name, current->info) == 0) {
			
			*target = (Dir *)current->info;
		
			return;
		}
	}

	//mutare in director anterior
	if (strcmp(name, "..") == 0) {
		if ((*target)->parent != NULL) {
			*target = (*target)->parent;
		}
		return;
	}

	//directorul nu se gaseste
	if (search((*target)->head_children_dirs, name, compare_directory) == 1) {
		printf("No directories found!\n");
	} 
	return;
}

//intoarce calea directorului
char *pwd (Dir* target) {

	Dir *current = target;

	int len, pos = 1, contor = 0;
	char *buffer = calloc(MAX_PATH_SIZE, sizeof(char));
	
	*buffer = '/';
	//parcurg prin parinte drumul pana la directorul home
	while (current != NULL) {
		//pastrez drumul in variabila buffer
		strcpy(buffer + pos, current->name);
		len = strlen(current->name);
		contor++;
		pos = pos + len;
		if (current->parent != NULL) {
			*(buffer + pos) = '/';
			pos++;
		}
		current = current->parent;
	}

	len = strlen(buffer);

	char *string = calloc(len + 1, sizeof(char));
	pos = 0;
	char *aux = calloc(len + 1, sizeof(char));
	//inversez variabila buffer si pastrez rezultatul in string
	for (int i = len - 1; i >= 0; i--) {
		if (buffer[i] == '/') {
			char *aux = calloc(len + 1, sizeof(char));
			strcpy(string + pos, buffer + i);

			pos = pos + strlen(buffer + i);

			strncpy(aux, buffer, len - strlen(buffer + i));
	
			strcpy(buffer, aux);

			len = strlen(buffer);
	
			free(aux);

		}
	}
	free(buffer);
	free(aux);
	
	return string;

}


//afiseaza sub forma arborescenta directoarele si fisierele
void tree(Dir *target, int level)
{	
	//daca directorul nu exista iese
	if (target == NULL){
		return;
	}

	//afiseaza numele directorului cu spatii
	if (level > 0) {
		if (level != 1) {
			int copy = (level - 1) * 4;
			while (copy ) {
				printf(" ");
				copy--;
			}
		
		}
		printf("%s\n", target->name);
	}

	//parcurge lista de directoare a directorului
	if (target->head_children_dirs != NULL) {
	
		List head = target->head_children_dirs;
		List current = head;

		//apeleaza recursiv functia pentru fiecare director
		for (; current != NULL; current = current->next) {
			tree((Dir *)(current->info), level + 1);
		}
		
	}

	//printeaza lista de fisiere din director pe nivel
	print_list_rec(target->head_children_files, print_filename, level);
}

//mutare prim element
void mv_first(List *head) {
	//daca lista este goala sau are un singur element
	if (*head == NULL || (*head)->next == NULL) {
		return;
	}
	List first = *head;
	List last = *head;

	//se parcurge lista pana la final
	while (last->next != NULL) {
		if (last->next == NULL) {
			break;
		}
		last = last->next;
	}

	//se schimnba inceputul de lista
	*head = first->next;
	first->next = NULL;
	last->next = first;
}

//mutare director care nu este cap de lista
void mv_dir(List head, char *oldname, char *newname) {
	List copy = head;
	List last = head;
	List ant = NULL;
	while (last->next != NULL) {
		if (last->next == NULL) {
			break;
		}
		last = last->next;
	}

	for (; copy != NULL; copy = copy->next) {
		if (strcmp(((Dir *)(copy->info))->name, oldname) == 0) {
			strcpy((((Dir *)(copy->info))->name), newname);
			ant->next = last;
			last->next = NULL;
		}
		ant = copy;
	}
}

//mutare fisier care nu este cap de lista
void mv_file(List head, char *oldname, char *newname) {
	List copy = head;
	List last = head;
	List ant = NULL;
	while (last->next != NULL) {
		if (last->next == NULL) {
			break;
		}
		last = last->next;
	}

	for (; copy != NULL; copy = copy->next) {
		if (strcmp(((File *)(copy->info))->name, oldname) == 0) {
			strcpy((((File *)(copy->info))->name), newname);
			ant->next = last;
			last->next = NULL;
		}
		ant = copy;
	}
}

//muta fisier sau director
void mv(Dir* parent, char *oldname, char *newname) {
	//daca fisierul/directorul existadeja
	if (search_directory(parent->head_children_dirs, newname) != NULL ||
	search_file(parent->head_children_files, newname) != NULL) {
		printf("File/Director already exists\n");
		return;
	}

	if (parent->head_children_dirs != NULL) {
		//daca directorul vechi este primul din lista
		if (strcmp(((Dir *)(parent->head_children_dirs->info))->name, oldname) == 0) {
			if (strcmp(((Dir *)(parent->head_children_dirs->info))->name, oldname) == 0) {
				strcpy(((Dir *)(parent->head_children_dirs->info))->name, newname);
				mv_first(&(parent->head_children_dirs));
			}
			return;
		}
		//directorul vechi nu este cap de lista
		if (search_directory(parent->head_children_dirs, oldname) != NULL) {
			Dir *directory = (Dir *)search_directory(parent->head_children_dirs, oldname);
			mv_dir(directory->head_children_dirs, oldname, newname);
			return;
		}
	}
	else if (parent->head_children_files != NULL) {
		//fisierul este la inceput
		if (strcmp(((File *)(parent->head_children_files->info))->name, oldname) == 0) {
			strcpy(((Dir *)(parent->head_children_files->info))->name, newname);
			mv_first(&(parent->head_children_files));
			return;
		}
		//fisierul nu este primul din lista 
		if (search_file(parent->head_children_files, oldname) != NULL) {
			File *file = (File *)search_directory(parent->head_children_files, oldname);
			mv_file(file->parent->head_children_files, oldname, newname);
			return;
		}
		
	}
	//daca fisierul/directorul nu exista
	if (search_directory(parent->head_children_dirs, oldname) == NULL || 
	search_file(parent->head_children_files, newname) == NULL) {
		printf("File/Director not found\n");
		return;
	}
	
}

//elibereaza memoria directorului
void stop(Dir *directory) {
	dez_dir(directory);
}

int main () {

	//linie citita
	char *line = (char *)calloc(MAX_INPUT_LINE_SIZE, sizeof(char));
	fgets(line, MAX_INPUT_LINE_SIZE, stdin);
	
	//directorul home
	Dir *home_directory = allocate_Dir();
	home_directory->parent = NULL;
	strcpy(home_directory->name, "home");

	//directorul curent
	Dir *current_directory = home_directory;


	do {
		//tipul comenzii
		char *command = strtok(line, " \n");

		if (strcmp(command, "touch") == 0) {
			char *file = strtok(NULL, "\n");
			touch(current_directory, file);
		}
		else if (strcmp(command, "ls") == 0) {
			ls(current_directory);
		}
		else if (strcmp(command, "mkdir") == 0) {
			char *directory = strtok(NULL, "\n");
			mkdir(current_directory, directory);
		}
		else if (strcmp(command, "rm") == 0) {
			char *file = strtok(NULL, "\n");
			rm(current_directory, file);
		}
		else if (strcmp(command, "rmdir") == 0) {
			char *directory = strtok(NULL, "\n");
			rmdir(current_directory, directory);
		}
		else if (strcmp(command, "cd") == 0) {
			char *directory = strtok(NULL, "\n");
			cd(&current_directory, directory, home_directory);
		}
		else if (strcmp(command, "pwd") == 0) {
			char *path = pwd(current_directory);
			printf("%s\n", path);
			free(path);
		}
		else if (strcmp(command, "tree") == 0) {
			tree(current_directory, 0);
		}
		else if (strcmp(command, "mv") == 0) {
			char *old = strtok(NULL, " ");
			char *new = strtok(NULL, "\n");
			mv(current_directory, old, new);
		}
	} while (fgets(line, MAX_INPUT_LINE_SIZE, stdin));
	
	free(line);
	stop(home_directory);


	return 0;
}
