import socketserver
from socket import socket
from typing import Tuple

import cryptography
import cryptography.hazmat
import cryptography.hazmat.primitives
import cryptography.hazmat.primitives.ciphers
import cryptography.hazmat.primitives.ciphers.algorithms
import cryptography.hazmat.primitives.ciphers.modes
import cryptography.hazmat.primitives.padding


def xor_bytes(a: bytes, b: bytes) -> bytes:
    return bytes(x ^ y for x, y in zip(a, b))

SocketAddress = Tuple[str, int]


class OracleServer(socketserver.ThreadingTCPServer):
    """
    Sets up a socketserver with the OracleRequestHandler.
    Accepts a tuple as server address with (host: str, port: int)
    and a 128-bit AES key in hex-format.
    The key defaults to "00000000000000000000000000000000".
    """
    # key = b'\xeeH\xe0\xf4\xd0c\xeb\xd8\xb87\x16\xd3\t\xfe\x87\xce'
    def __init__(
            self,
            server_address: SocketAddress,
            key: str = "0102030405060708090A0B0C0D0E0F10"
    ) -> None:
        super().__init__(server_address, OracleRequestHandler)
        self._key = bytes.fromhex(key)

    @staticmethod
    def start(host: str = "localhost", port: int = 18651) -> None:
        """
        Starts a new OracleServer by default on localhost:4200
        """

        with OracleServer((host, port)) as server:
            print(f"Server started on {host}:{port}. Press Ctrl+C to stop.")
            try:
                server.serve_forever()
            except KeyboardInterrupt:
                server.shutdown()
                server.server_close()


class OracleRequestHandler(socketserver.BaseRequestHandler):
    def __init__(
            self, request: socket, client_address: SocketAddress, server: OracleServer
    ) -> None:
        self.request: socket = request
        self.client_address: SocketAddress = client_address
        self.server: OracleServer = server

        self.setup()
        try:
            self.handle()
        finally:
            self.finish()

    def setup(self) -> None:
        self._ciphertext = self._recv_exactly(16)
        # print(f"Ciphertext: {self._ciphertext.hex()}")

        cipher = cryptography.hazmat.primitives.ciphers.Cipher(
            cryptography.hazmat.primitives.ciphers.algorithms.AES(self.server._key),
            cryptography.hazmat.primitives.ciphers.modes.ECB(),
        )
        decryptor = cipher.decryptor()
        self._plaintext = decryptor.update(self._ciphertext) + decryptor.finalize()

    def handle(self) -> None:
        try:
            # Constantly handle q-block requests by client
            while True:
                self._receive_q_blocks()
        except (ConnectionError, ConnectionResetError):
            pass

    def finish(self) -> None:
        self.request.close()

    def _receive_q_blocks(self) -> None:
        expectedBlocks = int.from_bytes(self._recv_exactly(2), "little")
        # print(f"Expected blocks: {expectedBlocks}")
        if expectedBlocks == 0:
            self.request.close()

        response = b""
        for i in range(expectedBlocks):
            # Receive all blocks and check each block for correct padding
            block = self._recv_exactly(16)

            if self._check_block(block):
                #print(f"Found valid padding at {i}")
                response += b"\x01"
            else:
                response += b"\x00"
        # print(f"Response: {response.hex()}")
        self.request.sendall(response)

    def _recv_exactly(self, n: int) -> bytes:
        data = b""
        while len(data) < n:
            packet = self.request.recv(n - len(data))
            if not packet:
                raise ConnectionError("Connection closed unexpectedly")
            data += packet
        return data

    def _check_block(self, block: bytes) -> bool:
        plaintext_xor = xor_bytes(block, self._plaintext)
        padder = cryptography.hazmat.primitives.padding.PKCS7(128).unpadder()
        padder.update(plaintext_xor)
        try:
            padder.finalize()
        except ValueError:
            return False
        return True


if __name__ == "__main__":
    OracleServer.start()