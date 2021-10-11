import styled, { createGlobalStyle } from 'styled-components'
import DirTree from './components/DirTrees';
const GlobalStyle = createGlobalStyle`
  * {
    padding: 0;
    margin: 0;
  }
`

const AppWrapper = styled.div`
  display: flex;
  justify-content: space-around;
  height: 100vh;
  &::first-child {
    visibility: hidden;
    background-color: red;
  }
  > div {
    width: 100%;
    padding: 10px 20px;
  }
`

function App() {
  
  return (
    <AppWrapper>
      <GlobalStyle />
      {/* <DirTree title='源文件' bgColor='#B5A28A' /> */}
      <DirTree title='数据备份' bgColor='#AB94CC' />
    </AppWrapper>
  );
}

export default App;
