#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char name[256];
    struct Node* next;
} Node;

Node* createNode(char* name) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->name, name);
    newNode->next = NULL;
    return newNode;
}

void insertNode(Node** head, Node* newNode) {
    if (*head == NULL) {
        *head = newNode;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void printList(Node* head) {
    Node* current = head;
    while (current != NULL) {
        printf("%s\n", current->name);
        current = current->next;
    }
}

void freeList(Node* head) {
    Node* current = head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
}

int main() {
    // Tạo linked list cho thư mục cha
    Node* rootList = NULL;
    insertNode(&rootList, createNode("File1.txt"));
    insertNode(&rootList, createNode("File2.txt"));
    insertNode(&rootList, createNode("Folder1"));
    insertNode(&rootList, createNode("Folder2"));

    // Tạo linked list cho thư mục con
    Node* subList = NULL;
    insertNode(&subList, createNode("File3.txt"));
    insertNode(&subList, createNode("File4.txt"));
    insertNode(&subList, createNode("Subfolder1"));

    // Liên kết linked list của thư mục con với linked list của thư mục cha
    Node* current = rootList;
    while (current != NULL && strcmp(current->name, "Folder2") != 0) {
        current = current->next;
    }
    if (current != NULL) {
        current->next = subList;
    }

    // In ra thông tin của linked list của thư mục cha
    printf("Linked list cua thu muc cha:\n");
    printList(rootList);

    // Giải phóng bộ nhớ đã cấp phát cho linked list
    freeList(rootList);

    return 0;
}