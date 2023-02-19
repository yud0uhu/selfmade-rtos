# MiniOS-AVR

## 簡易仕様

### timer_create()

- タイマーカウンターの作成

引数:unsigned int  
戻り値:void

### create_task()

- 新規タスクの作成

引数:unsigned char, unsigned int / Priority, void (\*task)(void)  
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

- プライオリティは 0~10 の範囲で指定

```

## main 関数

タイマーを用いる場合、main 関数の最後に下記の命令を明示する

```
while (1)
        ;
```

例

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

### 備考

#### タスク数

`#define TASK_ID_MAX 3` で上限タスクを 3 としている.
`TODO: 追加したタスクの数に応じてこの値は可変とする`

#### シミュレーション環境

[Tinkercad](https://www.tinkercad.com/things/2uc1RZ1pehi?sharecode=di9PmKFFPG8xi3DcQ0-Dp2gLGF1XviOcff3-7CuWrFw)
