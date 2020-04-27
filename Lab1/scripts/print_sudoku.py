def print_sudoku(board_str):
    print("\n======== board ========")

    for i in range(0, 9):
        row = ""
        for ch in board_str[i * 9 : (i + 1) * 9]:
            row = row + " " + ch
        print(row)

    print("=======================\n")

if __name__ == "__main__":
    while True:
        board_str = input()
        print_sudoku(board_str)
