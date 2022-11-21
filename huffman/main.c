#include "../clap/clap/clap.h"

typedef struct Node
{
	char c;
	size_t frequency;

	struct Node* left;
	struct Node* right;
} Node;

void huffman_print(Node* root, size_t tabs)
{
	if (root == NULL)
		return;

	for (size_t i = 0; i < tabs; i++)
		printf("\t");

	if (root->c)
		printf("Node: Value: '%c', Frequency: %zu\n", root->c, root->frequency);
	else
		printf("Node: Value: '', Frequency: %zu\n", root->frequency);

	huffman_print(root->left, tabs + 1);
	huffman_print(root->right, tabs + 1);
}

size_t str_different_characters_count(const char* str)
{
	if (str == NULL || strlen(str) < 1)
		return 0;
	
	uint8_t frequency_map[128];
	memset(frequency_map, 0, 128);

	for (size_t i = 0; i < strlen(str); i++)
		frequency_map[str[i]]++;

	size_t num_characters = 0;
	for (size_t i = 0; i < 128; i++)
		if (frequency_map[i] > 0)
			num_characters++;

	return num_characters;
}

Node* huffman_tree_from_text(const char* text)
{
	if (text == NULL || strlen(text) < 1)
		return NULL;
	
	size_t num_characters = str_different_characters_count(text);

	if (num_characters < 2)
	{
		Node* node = malloc(1 * sizeof(Node));
		node->c = text[0];
		node->frequency = strlen(text);
		node->left = NULL;
		node->right = NULL;
		return node;
	}

	size_t num_nodes = num_characters + (num_characters - 1);

	Node* nodes = calloc(num_nodes, sizeof(Node));

	uint8_t* nodes_is_parent = malloc(num_nodes * sizeof(uint8_t));
	memset(nodes_is_parent, 1, num_nodes);

	for (size_t i = 0; i < strlen(text); i++)
	{
		for (size_t j = 0; j < num_nodes; j++)
		{
			Node* node = &nodes[j];

			if (!node->c)
				node->c = text[i];

			if (node->c == text[i])
			{
				node->frequency++;
				break;
			}
		}
	}

	size_t least_left_index = 0;
	size_t least_right_index = 1;
	
	Node* least_left = &nodes[least_left_index];
	Node* least_right = &nodes[least_right_index];
	
	size_t num_parents = num_nodes;
	while (num_parents >= 2)
	{
		for (size_t i = 0; i < num_nodes; i++)
		{
			if (!nodes_is_parent[i])
				continue;
	
			Node* node = &nodes[i];

			if (node->frequency == 0)
				continue;

			if (node->frequency < least_left->frequency)
			{
				least_right = least_left;
				least_right_index = least_left_index;
	
				least_left = node;
				least_left_index = i;
			}
			else if (node != least_left && (least_right == NULL || node->frequency < least_right->frequency))
			{
				least_right = node;
				least_right_index = i;
			}
		}

		for (size_t i = 0; i < num_nodes; i++)
		{
			if (!nodes_is_parent[i])
				continue;
			
			Node* parent = &nodes[i];

			if (parent->c || parent->frequency > 0 || parent == least_left || parent == least_right)
				continue;

			parent->frequency = least_left->frequency + least_right->frequency;
			parent->left = least_left;
			parent->right = least_right;

			nodes_is_parent[least_left_index] = 0;
			nodes_is_parent[least_right_index] = 0;

			least_left = parent;
			least_right = NULL;

			least_left_index = i;

			num_parents -= 2;
			break;
		}
	}
	
	return least_left;
}

size_t huffman_branch_sum(Node* root)
{
	if (root->c)
		return 0;

	return root->frequency + huffman_branch_sum(root->left) + huffman_branch_sum(root->right);
}

size_t huffman_leaf_sum(Node* root)
{
	if (root->c)
		return root->frequency;

	return huffman_leaf_sum(root->left) + huffman_leaf_sum(root->right);
}

uint8_t huffman_encode_char(Node* root, char* dest, char c)
{
	if (root == NULL)
		return 0;

	if (root->c == c)
		return 1;

	size_t len = strlen(dest);

	if (huffman_encode_char(root->left, dest, c))
	{
		for (size_t i = 0; i < len; i++)
		{
			if (dest[len - 1 - i] == ' ')
			{
				dest[len - 1 - i] = '0';
				return 1;
			}
		}
	}
	else if (huffman_encode_char(root->right, dest, c))
	{
		for (size_t i = 0; i < len; i++)
		{
			if (dest[len - 1 - i] == ' ')
			{
				dest[len - 1 - i] = '1';
				return 1;
			}
		}
	}

	return 0;
}

const char* huffman_encode(Node* root, const char* text)
{
	size_t len = strlen(text);
	if (len < 2)
		return NULL;
	
	size_t branch_sum = huffman_branch_sum(root);

	char* encoded = malloc((branch_sum + 1) * sizeof(char));
	memset(encoded, ' ', branch_sum);
	encoded[branch_sum] = '\0';

	for (size_t i = 0; i < len; i++)
		huffman_encode_char(root, encoded, text[len - 1 - i]);

	return encoded;
}

size_t huffman_decode_char(Node* root, char* decoded, const char* encoded)
{
	if (root == NULL)
		return 0;
	
	if (root->c)
	{
		*decoded = root->c;
		return 0;
	}

	switch (*encoded)
	{
		case '0': return 1 + huffman_decode_char(root->left, decoded, encoded + 1);
		case '1': return 1 + huffman_decode_char(root->right, decoded, encoded + 1);
	}

	return 0;
}

const char* huffman_decode(Node* root, const char* encoded)
{
	size_t leaf_sum = huffman_leaf_sum(root);

	char* decoded = malloc((leaf_sum + 1) * sizeof(char));
	decoded[leaf_sum] = '\0';

	size_t offset = 0;
	for (size_t i = 0; i < leaf_sum; i++)
		offset += huffman_decode_char(root, decoded + i, encoded + offset);

	return decoded;
}

int main(int argc, char** argv)
{
	const char* text = NULL;
	uint8_t display_encoding = 0;
	uint8_t display_decoding = 0;
	
	clapRegisterFlag("help", 'h', CLAP_FLAG_OPT_ARG, NULL);
	clapRegisterFlag("encode", 'e', CLAP_FLAG_NO_ARG, NULL);
	clapRegisterFlag("decode", 'd', CLAP_FLAG_NO_ARG, NULL);

	while (clapParse(argc, argv))
	{
		if (clapParsedFlag("help", 'h'))
		{
			const char* arg = clapGetArg();

			if (arg == NULL)
				printf("[Huffman]: The following flags are supported:\n[Huffman]: --encode (-e)\n[Huffman]: --decode (-d)\n");
			else
			{
				if (strcmp(arg, "encode") == 0 || strcmp(arg, "e") == 0)
					printf("[Huffman]: --encode (-e): Enables to display the encoded string.\n");

				if (strcmp(arg, "decode") == 0 || strcmp(arg, "d") == 0)
					printf("[Huffman]: --decode (-d): Enables to display the decoded string.\n");
			}
		}

		if (clapParsedFlag("encode", 'e'))
			display_encoding = 1;

		if (clapParsedFlag("decode", 'd'))
			display_decoding = 1;

		const char* non_opt = NULL;
		if (clapParsedNonOpt(&non_opt) && non_opt != argv[0])
			text = non_opt;
	}

	if (text == NULL)
	{
		printf("[Huffman]: Missing string input for the algorithm!\n");
		return 1;
	}

	Node* root = huffman_tree_from_text(text);

	const char* encoded = huffman_encode(root, text);
	if (display_encoding)
		printf("Encoded: \"%s\"\n", encoded);

	if (display_decoding)
	{
		const char* decoded = huffman_decode(root, encoded);
		printf("Decoded: \"%s\"\n", decoded);
	}

	huffman_print(root, 0);

	return 0;
}