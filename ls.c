#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#define HALF_YEAR_SECOND (365 * 24 * 60 * 60 / 2);
#define PATH_MAX 4096

static time_t half_year_ago;
struct stat st;
//以下は参考プログラムからの引用
static void get_mode_string(mode_t mode, char *str) {
  str[0] = (S_ISBLK(mode))  ? 'b' :
           (S_ISCHR(mode))  ? 'c' :
           (S_ISDIR(mode))  ? 'd' :
           (S_ISREG(mode))  ? '-' :
           (S_ISFIFO(mode)) ? 'p' :
           (S_ISLNK(mode))  ? 'l' :
           (S_ISSOCK(mode)) ? 's' : '?';
  str[1] = mode & S_IRUSR ? 'r' : '-';
  str[2] = mode & S_IWUSR ? 'w' : '-';
  str[3] = mode & S_ISUID ? (mode & S_IXUSR ? 's' : 'S') : (mode & S_IXUSR ? 'x' : '-');
  str[4] = mode & S_IRGRP ? 'r' : '-';
  str[5] = mode & S_IWGRP ? 'w' : '-';
  str[6] = mode & S_ISGID ? (mode & S_IXGRP ? 's' : 'S') : (mode & S_IXGRP ? 'x' : '-');
  str[7] = mode & S_IROTH ? 'r' : '-';
  str[8] = mode & S_IWOTH ? 'w' : '-';
  str[9] = mode & S_ISVTX ? (mode & S_IXOTH ? 't' : 'T') : (mode & S_IXOTH ? 'x' : '-');
  str[10] = '\0';
}
//ここまで引用

//stackにnameをいれて、その中でsortさせて、stackの中身を取り出し、表示させた。
struct Stack {
  char *array[100000];
  int counter;
};

static void push_stack(struct Stack *stack,char *info){
  stack->array[stack->counter] = info;
  stack->counter++;
  if(stack->counter>=100000)
  {
    printf("Stack Overflow");
    exit(1);
  }
}
static int compare_name(const void *a, const void *b) {
   const char *a1 = *(const char **)a;
   const char *b1 = *(const char **)b;
   return strcmp(a1,b1);
}


static void sort_stack(struct Stack *stack) {
  qsort(stack->array, stack->counter, sizeof(char *), compare_name);
}


static void init_stack(struct Stack *stack) {
  stack->counter = 0;
}





//-lがあった場合の処理　内容は参考プログラムから引用した。
void loption(struct stat dent_stat){
  char buf[20];
  char mode_str[11];
  get_mode_string(dent_stat.st_mode,buf);
  printf("%s", buf);
  printf("%3d ", (int)dent_stat.st_nlink);

  struct passwd *passwd=getpwuid(dent_stat.st_uid);
  if(passwd!=NULL)
  {
    printf("%8s ", passwd->pw_name);
  } else{
    printf("%8d ",dent_stat.st_uid);
  }

  struct group *group=getgrgid(dent_stat.st_gid);
    if(group!=NULL)
    {
      printf("%8s", group->gr_name);
    }else{
      printf("%8d",dent_stat.st_gid);
    }

    if (S_ISCHR(dent_stat.st_mode) || S_ISBLK(dent_stat.st_mode)) {
      printf("%4d,%4d ", major(dent_stat.st_rdev), minor(dent_stat.st_rdev));
    } else {
      printf("%9lld ", dent_stat.st_size);
    }
     half_year_ago = time(NULL) - HALF_YEAR_SECOND;
    if(localtime(&dent_stat.st_mtim.tv_sec) - half_year_ago)
    {
      strftime(buf, sizeof(buf), "%m %d %H:%M", localtime(&dent_stat.st_mtim.tv_sec));
    }else{
      strftime(buf,sizeof(buf), "%m %d  %Y",localtime(&dent_stat.st_mtim.tv_sec) );
    }

     printf("%s  ", buf);


}
//ここまで引用

int main(int argc, char *argv[])
{
    DIR *dir;
    struct dirent *ent;
    char dirname[256];
    //nは調べるargv[]の添字
    int n=1;
    //kは-lオプションを使ったかどうかを0,1で表現
    int k=0;
    //fileは検索ファイルの指定があるかどうかの値
    int file=0;

    int result;

    char file_name[256];
    if (argc == 1) {
        snprintf(dirname,sizeof(dirname), ".");
        dir = opendir(dirname);
        if (dir == NULL) {
            fprintf(stderr, "unable to opendir %s\n", dirname);
            return 1;
        }
    } else {

      //-lオプションがあったかどうか検査
      if(*argv[1]=='-')
      {
        n++;
        k=1;
      }
      //argv[n]がディレクトリかファイルかを検査
       result = stat(argv[n],&st);

      if ((st.st_mode & S_IFMT) == S_IFDIR)
      {
        snprintf(dirname,sizeof(dirname),argv[n]);
        dir = opendir(dirname);
        n++;
        if (dir == NULL) {
            fprintf(stderr, "unable to opendir %s\n", dirname);
            return 1;
        }
      }else
      {
        snprintf(dirname,sizeof(dirname),"./");
        dir = opendir(dirname);
        if (dir == NULL) {
            fprintf(stderr, "unable to opendir %s\n", dirname);
            return 1;
      }

      }
      //検索ファイルがあるかどうかの検査
      if(argv[n]!=NULL)file=1;

    }
    struct Stack stack;
    init_stack(&stack);


    char path[PATH_MAX +1];
    int counter=0;


    while ((ent=readdir(dir)) != NULL) {
      char *name =ent->d_name;

      // "."や”..”の場合は出力しない
      if (ent->d_name[0] == '.') {
            continue;
      }
      //stat構造体に渡す
      size_t path_len=strlen(dirname);


      if (path_len >= PATH_MAX - 1) {
        fprintf(stderr, "too long path\n");
        return 0;
      }
      snprintf(path,PATH_MAX +1,"%s/%s",dirname,name);
      if (path[path_len - 1] != '/') {
        path[path_len] = '/';
        path_len++;
        path[path_len] = '\0';

      }
      push_stack(&stack,name);
      sort_stack(&stack);

    }


    int count=0;
    //以下改変
    //ファイル指定がある場合
    if(file==1)
    {
        for(int j=n;j<argc;j++)
        {
            struct stat tmp_stat;
            snprintf(path,PATH_MAX +1,"%s%s",dirname,argv[j]);
            if (stat(path, &tmp_stat) != 0) {
                  perror(path);
                  continue;
            }
                if(k==1)
                {
                   loption(tmp_stat);
                   printf("%s\n",argv[j]);
                }else
            {
                printf("%s\t",argv[j]);
            }
        }
        if(!k)printf("\n");
        return 0;
    }
    struct Stack *stacks=&stack;
      for(int i=0;i<stacks->counter;i++)
      {
        const char *name =stacks->array[i];
        struct stat dent_stat;
        snprintf(path,PATH_MAX +1,"%s/%s",dirname,name);

        if (stat(path, &dent_stat) != 0) {
          perror(path);
          continue;
        }
        if(k==1)
                {
                   loption(dent_stat);
                   printf("%s\n",name);
                }else
            {
                printf("%s\t",name);
            }

      if(k==0&&count%7==6)printf("\n");

      count++;
      }
      if(!k)printf("\n");




    closedir(dir);
    return 0;
