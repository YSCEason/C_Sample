// �U�C ifdef �϶��O�إߥ����H��U�q DLL �ץX���зǤ覡�C
// �o�� DLL �����Ҧ��ɮ׳��O�ϥΩR�O�C���ҩw�q APIJTAG_EXPORTS �Ÿ��sĶ���C
// �b�R�O�C�w�q���Ÿ��C����M�׳������w�q�o�ӲŸ�
// �o�ˤ@�ӡA��l�{���ɤ��]�t�o�ɮת������L�M��
// �|�N APIJTAG_API �禡�����q DLL �פJ���A�ӳo�� DLL �h�|�N�o�ǲŸ�����
// �ץX���C
#ifdef APIJTAG_EXPORTS
#define APIJTAG_API __declspec(dllexport)
#else
#define APIJTAG_API __declspec(dllimport)
#endif

// �o�����O�O�q ApiJtag.dll �ץX��
class APIJTAG_API CApiJtag {
public:
	CApiJtag(void);
	// TODO: �b���[�J�z����k�C
};

extern APIJTAG_API int nApiJtag;

APIJTAG_API int fnApiJtag(void);
