#!/bin/bash

# 確保 abc 可執行
if [ ! -x "./abc" ]; then
    echo "Error: ./abc 不存在或不可執行"
    exit 1
fi

# 定義案例範圍
cases=("case03")
# 定義參數組合
param_combinations=("0 0" "1 1" "1 0")

# 遍歷每個案例
for case in "${cases[@]}"; do
    # 建立該案例的路徑
    dir="release/$case"
    
    # 檢查必要檔案是否存在
    circuit_1="$dir/circuit_1.v.aig"
    circuit_2="$dir/circuit_2.v.aig"
    input_file="$dir/input"
    if [ ! -f "$circuit_1" ] || [ ! -f "$circuit_2" ] || [ ! -f "$input_file" ]; then
        echo "Warning: 跳過 $case，必要檔案缺失"
        continue
    fi

    # 對每個參數組合執行 ./abc
    for params in "${param_combinations[@]}"; do
        echo "正在執行案例: $case，參數: $params"
        # 使用管道自動向 abc 提供指令
        ./abc <<EOF
read_two_aig $circuit_1 $circuit_2 $input_file $params
quit
EOF
    done
done

echo "所有案例執行完畢！"
