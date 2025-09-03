
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100   // max size for stack and queue

// ======================= Structures =========================
struct Book
{
    int ISBN;
    char title[50];
    int available; // 1 = available, 0 = borrowed
    struct Book *left;
    struct Book *right;
};

struct Borrower
{
    int id;
    char name[50];
    struct Borrower *next;
};

// Queue (array-based for waiting list)
struct Queue
{
    int arr[MAX];
    int front, rear;
};

// Stack (array-based for undo - optional for librarian use)
struct Stack
{
    char action[MAX][20];
    int borrowerID[MAX];
    int ISBN[MAX];
    int top;
};

// ======================= BST Functions (Books) =========================
struct Book* insertBook(struct Book *root, int ISBN, char *title)
{
    if (root == NULL)
    {
        struct Book *newBook = (struct Book*)malloc(sizeof(struct Book));
        newBook->ISBN = ISBN;
        strcpy(newBook->title, title);
        newBook->available = 1;
        newBook->left = newBook->right = NULL;
        return newBook;
    }
    if (ISBN < root->ISBN) root->left = insertBook(root->left, ISBN, title);
    else if (ISBN > root->ISBN) root->right = insertBook(root->right, ISBN, title);
    return root;
}

struct Book* searchBook(struct Book *root, int ISBN)
{
    if (root == NULL || root->ISBN == ISBN) return root;
    if (ISBN < root->ISBN) return searchBook(root->left, ISBN);
    return searchBook(root->right, ISBN);
}

void inorderBooks(struct Book *root)
{
    if (root != NULL)
    {
        inorderBooks(root->left);
        printf("ISBN: %d | Title: %s | %s\n", root->ISBN, root->title,
               root->available ? "Available" : "Not Available");
        inorderBooks(root->right);
    }
}

// Find minimum in BST (for remove)
struct Book* findMin(struct Book* root)
{
    while (root && root->left != NULL)
        root = root->left;
    return root;
}

// Remove book by ISBN
struct Book* removeBook(struct Book* root, int ISBN)
{
    if (root == NULL) return root;

    if (ISBN < root->ISBN)
    {
        root->left = removeBook(root->left, ISBN);
    }
    else if (ISBN > root->ISBN)
    {
        root->right = removeBook(root->right, ISBN);
    }
    else
    {
        // Found book
        if (root->left == NULL)
        {
            struct Book* temp = root->right;
            free(root);
            return temp;
        }
        else if (root->right == NULL)
        {
            struct Book* temp = root->left;
            free(root);
            return temp;
        }
        // Two children: get inorder successor
        struct Book* temp = findMin(root->right);
        root->ISBN = temp->ISBN;
        strcpy(root->title, temp->title);
        root->available = temp->available;
        root->right = removeBook(root->right, temp->ISBN);
    }
    return root;
}

// ======================= Linked List Functions (Borrowers) =========================
struct Borrower* addBorrower(struct Borrower *head, int id, char *name)
{
    struct Borrower *newB = (struct Borrower*)malloc(sizeof(struct Borrower));
    newB->id = id;
    strcpy(newB->name, name);
    newB->next = NULL;

    if (head == NULL)
    {
        return newB;
    }

    struct Borrower *temp = head;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = newB;
    return head;
}


void displayBorrowers(struct Borrower *head)
{
    struct Borrower *temp = head;
    while (temp != NULL)
    {
        printf("ID: %d | Name: %s\n", temp->id, temp->name);
        temp = temp->next;
    }
}

// ======================= Queue Functions (Array-based) =========================
void initQueue(struct Queue *q)
{
    q->front = 0;
    q->rear = -1;
}

int isQueueEmpty(struct Queue *q)
{
    return q->front > q->rear;
}

int isQueueFull(struct Queue *q)
{
    return q->rear == MAX - 1;
}

void enqueue(struct Queue *q, int borrowerID)
{
    if (isQueueFull(q))
    {
        printf("Queue full! Cannot add to waiting list.\n");
        return;
    }
    q->arr[++q->rear] = borrowerID;
}

int dequeue(struct Queue *q)
{
    if (isQueueEmpty(q)) return -1;
    return q->arr[q->front++];
}

// ======================= Stack Functions (Array-based) =========================
void initStack(struct Stack *s)
{
    s->top = -1;
}

int isStackEmpty(struct Stack *s)
{
    return s->top == -1;
}

void push(struct Stack *s, char *action, int borrowerID, int ISBN)
{
    if (s->top == MAX - 1) return;
    s->top++;
    strcpy(s->action[s->top], action);
    s->borrowerID[s->top] = borrowerID;
    s->ISBN[s->top] = ISBN;
}
void displayHistory(struct Stack *s) {
    if (isStackEmpty(s)) {
        printf("No borrow/return history available.\n");
        return;
    }
    printf("\n--- Borrow/Return History ---\n");
    for (int i = s->top; i >= 0; i--) {
        printf("[%d] Action: %s | Borrower ID: %d | ISBN: %d\n",
               i + 1, s->action[i], s->borrowerID[i], s->ISBN[i]);
    }
}

// ======================= Core Functions =========================
void borrowBook(struct Book *root, int ISBN, int borrowerID,
                struct Queue *waitlist, struct Stack *undoStack)
{
    struct Book *b = searchBook(root, ISBN);
    if (b == NULL)
    {
        printf("Book not found!\n");
        return;
    }
    if (b->available)
    {
        b->available = 0;
        printf("Borrower %d borrowed book %s\n", borrowerID, b->title);
        push(undoStack, "borrow", borrowerID, ISBN);
    }
    else
    {
        printf("Book not available, added to waitlist.\n");
        enqueue(waitlist, borrowerID);
    }
}

void returnBook(struct Book *root, int ISBN,
                struct Queue *waitlist, struct Stack *undoStack)
{
    struct Book *b = searchBook(root, ISBN);
    if (b == NULL)
    {
        printf("Book not found!\n");
        return;
    }
    if (!b->available)
    {
        int nextBorrower = dequeue(waitlist);
        if (nextBorrower == -1)
        {
            b->available = 1;
            printf("Book %s returned and now available.\n", b->title);
        }
        else
        {
            printf("Book %s returned and assigned to borrower %d.\n", b->title, nextBorrower);
        }
        push(undoStack, "return", nextBorrower, ISBN);
    }
}

// ======================= Main =========================
int main()
{
    struct Book *root = NULL;
    struct Borrower *borrowers = NULL;
    struct Queue waitlist;
    struct Stack undoStack;

    initQueue(&waitlist);
    initStack(&undoStack);

    int choice, id, isbn;
    char name[50];
    char password[20];

    // Sample Data
    root = insertBook(root, 101, "C Programming");
    root = insertBook(root, 202, "Data Structures");
    root = insertBook(root, 303, "Algorithms");

    borrowers = addBorrower(borrowers, 1, "Alice");
    borrowers = addBorrower(borrowers, 2, "Bob");

    while (1)
    {
        printf("\n==== Welcome to Library System ====\n");
        printf("1. Librarian\n2. Customer\n3. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        if (choice == 1)
        {
            printf("Enter Librarian Password: ");
            scanf("%s", password);
            if (strcmp(password, "nfs123") != 0)
            {
                printf("Access Denied!\n");
                continue;
            }

            int lchoice;
            while (1)
            {
                printf("\n--- Librarian Menu ---\n");
                printf("1. Add Book\n2. Remove Book\n3. Display Books\n4. Add Borrower\n5. Show Borrower List\n6.Display Borrow History\n7. Back\n");
                //printf("1. Add Book\n2. Remove Book\n3. Display Books\n4. Add Borrower\n5. Show Borrower List\n6.\n6. Back\n");
                printf("Choice: ");
                scanf("%d", &lchoice);

                if (lchoice == 1)
                {
                    printf("Enter ISBN and Title: ");
                    scanf("%d %[^\n]", &isbn, name);
                    root = insertBook(root, isbn, name);
                }
                else if (lchoice == 2)
                {
                    printf("Enter ISBN to remove: ");
                    scanf("%d", &isbn);
                    root = removeBook(root, isbn);
                }
                else if (lchoice == 3)
                {
                    inorderBooks(root);
                }
                else if (lchoice == 4)
                {
                    printf("Enter Borrower ID and Name: ");
                    scanf("%d %[^\n]", &id, name);
                    borrowers = addBorrower(borrowers, id, name);
                }
                else if (lchoice == 5)
                {
                    displayBorrowers(borrowers);
                }
                else if (lchoice == 6) {
                    displayHistory(&undoStack);
                }
                else if(lchoice==7)
                {
                    break;
                }
            }
        }
        else if (choice == 2)
        {
            int cchoice;
            while (1)
            {
                printf("\n--- Customer Menu ---\n");
                printf("1. Display Books\n2. Borrow Book\n3. Return Book\n4. Back\n");
                printf("Choice: ");
                scanf("%d", &cchoice);

                if (cchoice == 1)
                {
                    inorderBooks(root);
                }
                else if (cchoice == 2)
                {
                    printf("Enter Borrower ID and Book ISBN: ");
                    scanf("%d %d", &id, &isbn);
                    borrowBook(root, isbn, id, &waitlist, &undoStack);
                }
                else if (cchoice == 3)
                {
                    printf("Enter Book ISBN: ");
                    scanf("%d", &isbn);
                    returnBook(root, isbn, &waitlist, &undoStack);
                }
                else if (cchoice == 4)
                {
                    break;
                }
            }
        }
        else if (choice == 3)
        {
            exit(0);
        }
    }

    return 0;
}

