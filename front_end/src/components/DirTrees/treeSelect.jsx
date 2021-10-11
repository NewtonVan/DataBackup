import { useState, useEffect } from 'react'
import styled from 'styled-components'
import { Menu, Dropdown, Modal, Input } from 'antd'
import { LeftOutlined, ReloadOutlined, MoreOutlined } from '@ant-design/icons'

import { getDirTrees, getCurrentPwd, takeAction } from '../../request'

const mockTree = [
  {
    filename: 'README.md',
  }, {
    filename: 'examples'
  }, {
    filename: 'package.json'
  }, {
    filename: 'test'
  }, {
    filename: 'azure-pipelines.yml'
  }, {
    filename: 'jest.config.js'
  }, {
    filename: 'packages'
  }, {
    filename: 'tsconfig.json'
  }
]

const TreeSelectWrapper = styled.div`
  padding: 20px 0;
`

const CurrentPathWrapper = styled.div`
  font-weight: bold;
  border: 1px solid black;
  padding: 10px 0;
  flex: 100% 1 1;
  display: flex;
  flex-direction: row;
  align-items: stretch;
  border-radius: 3px;
  background: #e4f5ef;
`

const ItemDir = styled.span`
  background-color: rgba(55,55,55, .1);
  display: inline-block;
  margin-left: 5px;
  padding: 0 8px;
  box-sizing: border-box;
`

const SubTreeWrapper = styled.ul`
  list-style: none;
  padding: 10px 20px;
  height: 500px;
  overflow-y: scroll;
  > li {
    &:hover {
      background-color: #DDD;
    }
    margin-top: 10px;
    padding: 0 8px;
    line-height: 30px;
    border-radius: 4px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
`
const SubDir = styled.li`
  span {
    width: 200px;
  }
`

const FileTypeWrapper = styled.span`
  color: ${({ filetype }) => {
    if (filetype === 'reg') return 'black'
    else if (filetype === 'dir') return '#345a81'
    else if (filetype === 'fifo') return '#479A5F'
    else if (filetype === 'symlink') return '#F04370'
    else return 'black'
  }};
`

export default function TreeSelect({
  initPath = '/'
}) {
  const [currentPath, setCurrentPath] = useState(initPath)
  const [currentTrees, setCurrentTrees] = useState(mockTree)

  useEffect(() => {
    getCurrentPwd().then((cwd) => {
      setCurrentPath(cwd)
    })
  }, [])

  useEffect(() => {
    getDirTrees(currentPath).then(res => setCurrentTrees(res))
  }, [currentPath])


  const changePath = (index) => {
    setCurrentPath(['',].concat(currentPath.split('/').filter(i => !!i).slice(0, index + 1)).join('/'))
  }
  const backPath = () => {
    const currentPathArr = currentPath.split('/').filter(i => !!i)
    if (currentPathArr.length <= 1) return
    setCurrentPath(['',].concat(currentPath.split('/').filter(i => !!i).slice(0, currentPathArr.length - 1)).join('/'))
  }
  const reload = () => {
    getDirTrees(currentPath).then(res => setCurrentTrees(res))
  }

  return (
    <TreeSelectWrapper>
      <CurrentPathWrapper>
        <ItemDir> <ReloadOutlined onClick={reload} /> </ItemDir>
        <ItemDir> <LeftOutlined onClick={backPath} /> </ItemDir>
        {
          currentPath.split('/')
            .filter(i => !!i)
            .map((dir, index) => <ItemDir onClick={() => changePath(index)}>{dir}</ItemDir>)
        }
      </CurrentPathWrapper>

      <SubTreeWrapper>
        {
          currentTrees.sort(({ filename }) => filename).map(({ filename, filetype, size, inode }) => (
            <SubSelect
              filename={filename.split('/').pop()}
              filetype={filetype}
              size={size}
              inode={inode}
              currentPath={currentPath}
              reload={reload}
              onDoubleClick={() => {
                if (filename.indexOf('.') === -1) {
                  setCurrentPath(`${currentPath}/${filename.split('/').pop()}`)
                }
              }
              }
            />
          ))
        }
      </SubTreeWrapper>
    </TreeSelectWrapper>
  )
}

function SubSelect({ filename, currentPath, onDoubleClick, filetype, size, inode, reload }) {
  const [modalVis, setModalVis] = useState(false)
  const [action, setAction] = useState('')
  const [targetFile, setTargetFile] = useState('')
  const [passKey, setPassKey] = useState('')

  const menu = (
    <Menu>
      <Menu.Item onClick={() => { setModalVis(true); setAction('copy') }}>备份</Menu.Item>
      <Menu.Item onClick={() => { setModalVis(true); setAction('pack') }}>打包</Menu.Item>
      <Menu.Item onClick={() => { setModalVis(true); setAction('unpack') }}>解包</Menu.Item>
      <Menu.Item onClick={() => { setModalVis(true); setAction('compress') }}>压缩</Menu.Item>
      <Menu.Item onClick={() => { setModalVis(true); setAction('uncompress') }}>解压</Menu.Item>
      <Menu.Item onClick={() => { setModalVis(true); setAction('encrypt') }}>加密</Menu.Item>
      <Menu.Item onClick={() => { setModalVis(true); setAction('decrypt') }}>解密</Menu.Item>
    </Menu>
  );

  const handleAction = () => {
    takeAction(action, `${currentPath}/${filename}`, targetFile, passKey)
    reload()
    setModalVis(false)
  }
  return (
    <SubDir filetype={filetype} onDoubleClick={onDoubleClick} >
      <span>{filename}</span>
      <FileTypeWrapper filetype={filetype}>TYPE: {filetype}</FileTypeWrapper>
      <span>SIZE: {size}B</span>
      <span>INODE：{inode}</span>
      <Dropdown overlay={menu} trigger='click' placement='bottomRight'>
        <MoreOutlined />
      </Dropdown>
      <Modal title={action} visible={modalVis} onOk={handleAction} onCancel={() => { setModalVis(false) }}>
        {/* <Input placeholder="请输入源地址" defaultValue={`${currentPath}/${filename}`} onChange={(e) => { sourceFile(e.target.value) }} /> */}
        <Input placeholder="请输入目的地址" onChange={(e) => { setTargetFile(e.target.value) }}/>
        {
          (action.indexOf('crypt') !== -1) && 
          <Input placeholder="请输入密码" onChange={(e) => { setPassKey(e.target.value) }}/> 
        }
      </Modal>
    </SubDir>
  )
}