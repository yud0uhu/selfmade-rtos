# MiniOS-AVR

## 簡易仕様

### timer_create()

- タイマーカウンターの作成

引数:unsigned int  
戻り値:void

### create_task()

- 新規タスクの作成

引数:unsigned char, unsigned int, void (\*task)(void)  
戻り値:void

### delete_task()

- タスクの削除

引数:unsigned char  
戻り値:void

### タスクの作成

下記のように実装する

```c++
void task_function(void)
{
// do somethin
}

```

## main 関数

タイマーを用いる場合、main 関数の最後に下記の命令を明示する

```
while (1)
        ;
```

```c++
int main(void)
{
    create_task(0, 3, task_function);
    ...
    os_start();

    timer_create(62500); /* 任意のtick */
    while (1)
        ; // 無限ループ（割込み待ち）
}

```
