package create

import (
	"bufio"
	"fmt"
	"libvirt.org/go/libvirt"
	"os"
)

const (
	XML_DIR = "/home/jam/Desktop/libvirt"
)

func ReadFileContext(filename string) (string, error) {
	file_path := XML_DIR + "/" + filename
	file, err := os.Open(file_path)
	if err != nil {
		fmt.Println(err)
		return "", err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	str := ""
	for scanner.Scan() {
		str = str + scanner.Text()
	}

	if err := scanner.Err(); err != nil {
		fmt.Println(err)
		return "", err
	}
	return str, nil
}

func CreateVm(filename string) error {
	conn, err := libvirt.NewConnect("qemu:///system")
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		return err
	}
	defer conn.Close()
	xml_content, err_inform := ReadFileContext(filename)
	if err_inform != nil {
		return err_inform
	}
	dom, err := conn.DomainCreateXML(xml_content, 0)
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		return err
	}
	name, err := dom.GetName()
	if err == nil {
		fmt.Printf("  %s\n", name)
	}
	dom.Free()
	return nil
}

func CreateVmByXmlContext(xmlContent string) error {
	conn, err := libvirt.NewConnect("qemu:///system")
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		return err
	}
	defer conn.Close()
	dom, err := conn.DomainCreateXML(xmlContent, 0)
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		return err
	}
	name, err := dom.GetName()
	if err == nil {
		fmt.Printf("  %s\n", name)
	}
	dom.Free()
	return nil
}
