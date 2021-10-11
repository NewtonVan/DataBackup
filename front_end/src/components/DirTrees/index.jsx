import styled from "styled-components"
import TreeSelect from "./treeSelect"

const OptionsWrapper = styled.div`
  background-color: ${props => (props.bgColor || '#DDD')};
`

export default function DirTree({ title = '目录', bgColor }) {
  return (
    <OptionsWrapper bgColor={bgColor}>
      <h2>{title}</h2>
      <TreeSelect ></TreeSelect>
    </OptionsWrapper>
  )
}